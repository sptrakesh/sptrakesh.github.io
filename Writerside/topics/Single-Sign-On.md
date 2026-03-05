# Single Sign On

Some experiences implementing SSO using OIDC/OAuth2 [protocols](https://developer.okta.com/blog/2017/07/25/oidc-primer-part-1).
I recently had to implement SSO against Azure AD and Google Workspace for my company.  
The web frontend was built using [Wt](https://www.webtoolkit.eu/wt), while the backend API is 
built using [nghttp2_asio](nghttp2-framework.md).

## Workflow
I adopted the same workflow for implementing *authentication* against external
identity providers.

* Configure the OIDC/OAuth2 information in [config-db](Config-Db.md)
* Update our SSO Wt app to shows buttons for the supported IDPs.
* Create an *OIDCInformation* transient object in our database using [mongo-service](mongo-service.md).  This
  is used as the *nonce* value sent to the IDP.
* Redirect the user to the IDP for authentication.
* In the callback handler, perform the following steps:
  * Retrieve the OIDCInformation object from the database.
  * Exchange the code for an access token.
  * Use the access token to retrieve the user info.
  * Create an internal *User* object if not already present.
    * Leave the `password` empty for this user to prevent internal login flow.
  * Create a JWT for the user.  Update the *OIDCInformation* object with the JWT.
  * Redirect the user back to the Wt app.
* The redirect from the backend API is handled by a resource handler in the Wt app. 
  * Retrieves the **OIDCInformation** object from the database.
  * Retrieves the JWT using the **OIDCInformation** object.
  * Create a session and cookie.
  * Redirect the user to the Wt UI or other application that initiated the authentication flow.

Using the internal JWT avoids a lot of complexity in the backend API.  We do
not need to implement multiple token validation strategies.  All UI logic implemented using our
internal JWT remains unchanged.  In addition, we do not need to store the IDP access token,
refresh token, etc., since we do not need them to call directly into IDP APIs.