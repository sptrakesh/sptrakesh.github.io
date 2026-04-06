# Microsoft Entra OAuth2
<show-structure for="chapter,tab"/>

We originally planned to use the OIDC protocol, however, our customer had not enabled
OIDC, and hence we had to fall back to OAuth2.

These are the steps I followed to implement OAuth2 authentication against Microsoft Entra.

## Authentication Flow
I followed the steps [documented](https://learn.microsoft.com/en-us/entra/identity-platform/v2-oauth2-auth-code-flow)
and had very little trouble getting the authentication flow to work.  I was able to complete the
entire setup and development in an hour, building on the code developed for [Google OAuth2](Google-Workspace-OAuth2.md).

### Wt Application
I added a new button for Microsoft Entra in the Wt application.

When the user clicks the Microsoft Entra button, we read the required information from the
configuration database and redirect the user to the Microsoft Entra authentication page.

<tabs id="entra-wt">
  <tab title="Configuration" id="entra-configuration">
    <code-block lang="C++" collapsible="true">
    <![CDATA[
    template<size_t N>
    struct StringLiteral
    {
      constexpr StringLiteral( const char (&str)[N] )
      {
        std::copy_n(str, N, value);
      }

      char value[N];
    };

    template <StringLiteral idp>
    struct IdpHolder
    {
      static const IdpHolder& instance()
      {
        static IdpHolder holder;
        return holder;
      }

      std::string clientId;
      std::string callback;
      std::string scope;
      std::string authorise;

    private:
      IdpHolder()
      {
        using std::operator""s;
        const auto keys = std::vector{ std::format( "/service/sso/oidc/{}/clientId", idp.value ),
          std::format( "/service/sso/oidc/{}/callback", idp.value ),
          std::format( "/service/sso/oidc/{}/scope", idp.value ),
          std::format("/service/sso/oidc/{}/authorise", idp.value )
        };

        const auto svkeys = keys | std::views::transform( []( auto& k ) { return std::string_view{ k }; } ) | std::ranges::to<std::vector>();

        const auto results = util::Configuration::instance().get( svkeys );
        if ( results.size() != keys.size() )
        {
          LOG_CRIT << "Error retrieving OIDC configuration for IDP " << idp.value;
          throw std::runtime_error{ "OIDC configuration error" };
        }

        clientId = results[0].value_or( ""s );
        callback = results[1].value_or( ""s );
        scope = results[2].value_or( ""s );
        authorise = results[3].value_or( ""s );
      }
    };
    ]]>
    </code-block>
  </tab>
  <tab title="OIDC Information" id="entra-oidc-model">
    <code-block lang="C++" collapsible="true">
    <![CDATA[
    std::expected<model::OIDCInformation, std::string> create( std::string_view idp, const ui::Application& app )
    {
      using O = std::expected<model::OIDCInformation, std::string>;

      auto resp = O{ std::in_place };
      auto& model = resp.value();
      model.idp = idp;
      model.metadata.created = std::chrono::system_clock::now();
      model.metadata.modified = model.metadata.created;
      model.metadata.user.id = bsoncxx::oid{ "5f70ee572fc09200086c8f23" };
      model.metadata.user.username = "spt";
      if ( !app.getRedirectTo().empty() ) model.redirectTo = app.getRedirectTo();

      const auto [status, _] = db::create( model, ""sv, true );
      if ( status != 200 )
      {
        LOG_WARN << "Error creating OIDC information";
        return O{ std::unexpect, "Failed to create OIDC information" };
      }

      return resp;
    }
    ]]>
    </code-block>
  </tab>
  <tab title="Event Handler" id="entra-wt-handler">
    <code-block lang="C++" collapsible="true">
    <![CDATA[
  auto app = ui::Application::instance();
  auto model = psso::create( "exelon"sv, *app );
  if ( !model.has_value() )
  {
    error->setText( model.error() );
    error->setHidden( false );
    return;
  }

  const auto& holder = psso::IdpHolder<"entra">::instance();
  auto endpoint = boost::urls::url( holder.authorise );
  auto params = endpoint.params();
  params.append( { "response_type", "code" } );
  params.append( { "client_id", holder.clientId } );
  params.append( { "response_mode", "form_post" } );

  auto encrypted = util::Configuration::instance().encrypt( model.value().id.to_string() );
  boost::algorithm::replace_all( encrypted, "+", ":" );
  auto encoded = boost::urls::param_pct_view( "state", encrypted );
  endpoint.encoded_params().append( encoded );

  encoded = boost::urls::param_pct_view( "redirect_uri", holder.callback );
  endpoint.encoded_params().append( encoded );

  encoded = boost::urls::param_pct_view( "scope", holder.scope );
  endpoint.encoded_params().append( encoded );

  app->redirect( endpoint.c_str() );
  app->quit();
    ]]>
    </code-block>
  </tab>
</tabs>

We create a new OIDC information record in the database and redirect the user to the Microsoft Entra
authentication page.  We use the encrypted value of the document ID as the state parameter.  In
the callback, we decrypt the state parameter and use it to retrieve the OIDC information record.

### Callback Handler
The callback handler is implemented as a hidden endpoint in our API.  The handler performs the following tasks:
* Parse the form data sent by Microsoft Entra.
* Retrieve the OIDC information record from the database.
* Exchange the authorisation code for an access token.
* Create an internal user if not existing.
* Generate a JWT token for the internal user.
* Redirect the user to Wt application with updated OIDC information.

<tabs id="entra-callback">
  <tab title="Form Data" id="entra-callback-data">
    Parse the form data sent by Microsoft Entra.
    <code-block lang="C++" collapsible="true">
    <![CDATA[
        std::string code{};
        std::string state{};

        auto pairs = spt::util::split<std::string_view>( payload, 3, "&" );
        for ( auto& pair : pairs )
        {
          auto parts = spt::util::split<std::string_view>( pair, 2, "=" );
          if ( parts.size() != 2 ) continue;
          if ( parts[0] == "code" ) code = parts[1];
          else if ( parts[0] == "state" )
          {
            const auto decoded = boost::urls::decode_view( parts[1] );
            state = std::string{ decoded.begin(), decoded.end() };
          }
        }

        if ( state.empty() || code.empty() )
        {
          LOG_WARN << "Missing state or code parameter. APM id: " << apm.id;
          return error( 400, "Missing parameters"sv, nullptr, methods, req.header, apm );
        }
    ]]>
    </code-block>
  </tab>
  <tab title="OIDC Information" id="entra-callback-oidc-info">
    Retrieve the OIDC information record from the database.
    <code-block lang="C++" collapsible="true">
    <![CDATA[
    std::expected<model::OIDCInformation, std::string> retrieveOidc( const std::string& value, spt::ilp::APMRecord& apm )
    {
      using O = std::expected<model::OIDCInformation, std::string>;

      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
      cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, "Retrieve OIDC information" );
      DEFER( spt::ilp::setDuration( cp ) );
      auto idx = apm.processes.size();

      auto cstate = boost::algorithm::replace_all_copy( value, ":", "+" );
      auto id = util::Configuration::instance().decrypt( cstate );
      auto oid = spt::util::parseId( id );
      if ( !oid )
      {
        LOG_INFO << "Invalid state parameter " << id << ". APM id: " << apm.id;
        return O{ std::unexpect, "Invalid state parameter" };
      }

      WRAP_CODE_LINE( auto [status, oidc] = db::retrieve<model::OIDCInformation>( *oid, ""sv, apm ); )
      if ( status != 200 || !oidc )
      {
        LOG_WARN << "Error retrieving OIDC information for oid " << *oid << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving OIDC information" );
        return O{ std::unexpect, "Error retrieving OIDC information" };
      }

      return O{ std::in_place, std::move( *oidc ) };
    }

    auto oidc = retrieveOidc( state, apm );
    if ( !oidc.has_value() ) return error( 417, oidc.error(), nullptr, methods, req.header, apm );
    ]]>
    </code-block>
  </tab>
  <tab title="IDP Configuration" id="entra-callback-oidc-config">
  Retrieve IDP configuration from configuration database.
    <code-block lang="C++" collapsible="true">
    <![CDATA[
    template <StringLiteral idp>
    struct IdpHolder
    {
      static const IdpHolder& instance()
      {
        static IdpHolder holder;
        return holder;
      }

      std::string clientId;
      std::string clientSecret;
      std::string tokenEndpoint;
      std::string userInfo;
      std::string redirectUri;
      std::string success;

    private:
      IdpHolder()
      {
        const auto keys = std::vector{ std::format( "/service/sso/oidc/{}/clientId", idp.value ),
          std::format( "/service/sso/oidc/{}/clientSecret", idp.value ),
          std::format( "/service/sso/oidc/{}/token", idp.value ),
          std::format( "/service/sso/oidc/{}/userInfo", idp.value ),
          std::format( "/service/sso/oidc/{}/callback", idp.value ),
          std::format( "/service/sso/oidc/{}/success", idp.value )
        };

        const auto svkeys = keys | std::views::transform( []( auto& k ) { return std::string_view{ k }; } ) | std::ranges::to<std::vector>();
        const auto results = util::Configuration::instance().get( svkeys );
        if ( results.size() != keys.size() )
        {
          LOG_CRIT << "Error retrieving OIDC configuration";
          throw std::runtime_error{ "OIDC configuration error" };
        }

        clientId = results[0].value_or( ""s );
        clientSecret = util::Configuration::instance().decrypt( results[1].value_or( ""s ) );
        tokenEndpoint = results[2].value_or( ""s );
        userInfo = results[3].value_or( ""s );
        redirectUri = results[4].value_or( ""s );
        success = results[5].value_or( ""s );
      }
    };
    ]]>
    </code-block>
  </tab>
  <tab title="OIDC Tokens" id="entra-callback-oidc-tokens">
  Model that represents the tokens retrieved from the IDP.
    <code-block lang="C++" collapsible="true">
    <![CDATA[
    struct OIDCTokens
    {
      BEGIN_VISITABLES(OIDCTokens);
      VISITABLE(std::string, access_token);
      VISITABLE(std::string, scope);
      VISITABLE(std::string, token_type);
      VISITABLE(std::string, id_token);
      VISITABLE(std::string, refresh_token);
      VISITABLE_DIRECT_INIT(int64_t, expires_in, {0});
      END_VISITABLES;
    };
    ]]>
    </code-block>
  </tab>
  <tab title="Token Exchange" id="entra-callback-oidc-exchange">
  Exchange the <code>code</code> returned by Entra for tokens.
    <code-block lang="C++" collapsible="true">
    <![CDATA[
        const auto& holder = IdpHolder<"exelon">::instance();
        auto response = cpr::Post( cpr::Url{ holder.tokenEndpoint },
          cpr::Payload{ { "client_id", holder.clientId },
          { "scope", "openid email profile" },
          { "code", code },
          { "redirect_uri", holder.redirectUri },
          { "grant_type", "authorization_code" },
          { "client_secret", holder.clientSecret } } );

        if ( response.status_code != 200 )
        {
          LOG_WARN << "Error retrieving OIDC token. Status code: " << int(response.status_code) << ". " << response.text << ". APM id: " << apm.id;
          cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving OIDC token" );
          return error( 417, "Error retrieving OIDC token"sv, nullptr, methods, req.header, apm );
        }

        auto tokens = spt::util::json::unmarshall<OIDCTokens>( response.text );
    ]]>
    </code-block>
  </tab>
  <tab title="User Info" id="entra-callback-oidc-userinfo"> 
    The JWT token returned by Entra does not include the user's given or family names.  Hence, we need to use the *userInfo* endpoint to retrieve the user's name.
    <code-block lang="C++" collapsible="true">
    <![CDATA[
         const auto response = cpr::Get( cpr::Url{ holder.userInfo }, cpr::Bearer{ tokens.access_token } );
        if ( response.status_code != 200 )
        {
          LOG_WARN << "Error retrieving user info. Status code: " << int(response.status_code) << ". " << response.text << ". APM id: " << apm.id;
          cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving OIDC token" );
          return error( 417, "Error retrieving user info"sv, nullptr, methods, req.header, apm );
        }

        auto model = spt::util::json::unmarshall<EntraUser>( response.text );
        model.username = claims.preferred_username;
    ]]>
    </code-block>
  </tab>
  <tab title="Platform User" id="entra-callback-oidc-user"> 
    Create/update the platform user as appropriate.  If the account initially existed as an internal user, we remove the `password` to ensure the user can only login via the external IDP.
    <code-block lang="C++" collapsible="true">
    <![CDATA[
        WRAP_CODE_LINE( auto [_, user] = db::retrieve<model::User>( "email", model.email, apm, true ); )
        if ( !user )
        {
          WRAP_CODE_LINE( auto u = createUser( model, *oidc, apm ); )
          if ( !u.has_value() ) return error( 417, u.error(), nullptr, methods, req.header, apm );
          user.emplace( std::move( u.value() ) );
        }
        else
        {
          LOG_INFO << "User " << model.email << " already exists. APM id: " << apm.id;
          WRAP_CODE_LINE( removePassword( *user, apm ); )
        }
    ]]>
    </code-block>
  </tab>
  <tab title="Generate JWT" id="entra-callback-oidc-jwt"> 
    Generate a JWT for the user.  This will be used by all our applications to interact with our API.
    <code-block lang="C++" collapsible="true">
    <![CDATA[
    std::expected<model::Token, std::string> createToken( const model::User& user, model::OIDCInformation& oidc, spt::ilp::APMRecord& apm )
    {
      using O = std::expected<model::Token, std::string>;

      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
      cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, "Create JWT Token" );
      DEFER( spt::ilp::setDuration( cp ) );
      auto idx = apm.processes.size();

      auto token = util::generateToken( user, std::chrono::duration_cast<std::chrono::minutes>( std::chrono::hours{ 24 } ), model::JwtToken::Type::idp );
      WRAP_CODE_LINE( const auto tstatus = db::create( token, apm, true ); )
      if ( tstatus != 200 )
      {
        LOG_WARN << "Error creating token for user " << user.username << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, model::ilp::value::APM_DATABASE_ERROR );
        return O{ std::unexpect, "Error creating token" };
      }

      db::impl::clearSessions( user, token, apm );

      oidc.jwtId = token.id;
      oidc.metadata.modified = std::chrono::system_clock::now();
      WRAP_CODE_LINE( const auto _os = db::update( oidc, apm ); )
      if ( _os != 200 )
      {
        LOG_WARN << "Error updating OIDC info with jwt id " << token.id << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error updating OIDC info" );
        return O{ std::unexpect, "Error updating OIDC info" };
      }

      return O{ std::in_place, std::move( token ) };
    }

    WRAP_CODE_LINE( auto token = createToken( *user, *oidc, apm ); )
    if ( !token.has_value() ) return error( 417, token.error(), nullptr, methods, req.header, apm );

    ]]>
    </code-block>
  </tab>
  <tab title="Redirect" id="entra-callback-oidc-redirect"> 
    Redirect the user back to the Wt application.
    <code-block lang="C++" collapsible="true">
    <![CDATA[
        auto endpoint = boost::urls::url( holder.success );
        auto encoded = boost::urls::param_pct_view( "nonce", state );
        endpoint.encoded_params().append( encoded );

        LOG_INFO << "Writing response for " << req.path << "; redirecting to " << endpoint.c_str() << ". APM id: " << apm.id;
        auto resp = Response{ req.header };
        resp.headers.clear();
        resp.headers.emplace( "location", endpoint.c_str() );
        resp.headers.emplace( "content-length", "0" );
        resp.status = 302;
        resp.jwt = std::make_shared<model::JwtToken>( token.value().token() );
        resp.body.clear();
        resp.compressed = false;
        resp.correlationId = correlationId( req );
        resp.entity = model::OIDCInformation::EntityType();
        return resp;
    ]]>
    </code-block>
  </tab>
</tabs>
