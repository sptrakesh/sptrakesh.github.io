# APM Sample Data
Sample data extracted from Grafana to illustrate some of the information captured from one of our REST
API services.

Image below shows a panel set up on our Grafana instance.  We set up some dynamic variables to allow
us to quickly filter the metrics being displayed as appropriate.

<img src="apm-grafana.png" alt="Grafana Panel"/>

## Parent Record
A sample of parent records generated from a service is shown below.

| id                       | application   | entity        | host       | request_method | response_status | duration | city    | country       | request_path     | username | role      | timestamp               |
|--------------------------|---------------|---------------|------------|----------------|-----------------|----------|---------|---------------|------------------|----------|-----------|-------------------------|
| 67e482e1ed5538904404c883 | inventory-api | InventoryItem | stage-v2-1 | GET            | 200             | 7898004  | Chicago | United States | /inventory/item/ | rakesh   | superuser | 2025-03-26 17:42:41.092 |
| 67e482cfed5538904404c882 | inventory-api | Facility      | stage-v2-1 | GET            | 200             | 4526037  | Chicago | United States | /facility        | rakesh   | superuser | 2025-03-26 17:42:23.301 |
| 67e482c8ed5538904404c880 | inventory-api | Manufacturer  | stage-v2-1 | GET            | 200             | 94633057 | Chicago | United States | /manufacturer/   | rakesh   | superuser | 2025-03-26 17:42:16.740 |

The table panel in Grafana was set up with a simple query like the following:

<code-block lang="SQL">
select id, application, entity, customer, host, request_method, response_status, duration, city, country, 
  request_path, username, role, timestamp
from webapm
where type is null
and $__timeFilter(timestamp)
order by timestamp desc
</code-block>

## Process Records
A sample of the process records for a parent APM record is shown below.

<table class="table table-bordered table-hover table-condensed">
<thead><tr><th title="Field #1">id</th>
<th title="Field #2">type</th>
<th title="Field #3">file</th>
<th title="Field #4">function</th>
<th title="Field #5">line</th>
<th title="Field #6">caller_file</th>
<th title="Field #7">caller_function</th>
<th title="Field #8">caller_line</th>
<th title="Field #9">note</th>
<th title="Field #10">duration</th>
<th title="Field #11">timestamp</th>
</tr></thead>
<tbody><tr>
<td>67e482c8ed5538904404c880</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/global.hpp</td>
<td>wirepulse::http::Response wirepulse::http::global::retrieveAll(const spt::http2::framework::Request&amp;, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with Model = wirepulse::model::Manufacturer; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;)]</td>
<td align="right">335</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/manufacturer.cpp</td>
<td>wirepulse::http::addManufacturerRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:71&amp;&amp;)&gt; [with auto:71 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">142</td>
<td>List entities Manufacturer</td>
<td align="right">94608741</td>
<td>2025-03-26 17:42:16.740</td>
</tr>
<tr>
<td>67e482c8ed5538904404c880</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::EntitiesQueryResponse wirepulse::http::parseQuery(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;, const std::vector&lt;std::basic_string_view&lt;char&gt;, std::allocator&lt;std::basic_string_view&lt;char&gt; &gt; &gt;&amp;)</td>
<td align="right">183</td>
<td>/opt/wirepulse/inventory-api/src/http/global.hpp</td>
<td>wirepulse::http::Response wirepulse::http::global::retrieveAll(const spt::http2::framework::Request&amp;, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with Model = wirepulse::model::Manufacturer; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;)]</td>
<td align="right">343</td>
<td> </td>
<td align="right">1007</td>
<td>2025-03-26 17:42:16.740</td>
</tr>
<tr>
<td>67e482c8ed5538904404c880</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">84</td>
<td>/opt/wirepulse/inventory-api/src/http/global.hpp</td>
<td>wirepulse::http::Response wirepulse::http::global::retrieveAll(const spt::http2::framework::Request&amp;, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with Model = wirepulse::model::Manufacturer; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;)]</td>
<td align="right">352</td>
<td> </td>
<td align="right">35761315</td>
<td>2025-03-26 17:42:16.740</td>
</tr>
<tr>
<td>67e482c8ed5538904404c880</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">65</td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td align="right">35754715</td>
<td>2025-03-26 17:42:16.740</td>
</tr>
<tr>
<td>67e482c8ed5538904404c880</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">159</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">72</td>
<td>Retrieve token</td>
<td align="right">35750480</td>
<td>2025-03-26 17:42:16.740</td>
</tr>
<tr>
<td>67e482c8ed5538904404c880</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">101</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">171</td>
<td>Retrieve JWT token</td>
<td align="right">35692425</td>
<td>2025-03-26 17:42:16.740</td>
</tr>
<tr>
<td>67e482c8ed5538904404c880</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">113</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">114</td>
<td> </td>
<td align="right">35676100</td>
<td>2025-03-26 17:42:16.740</td>
</tr>
<tr>
<td>67e482c8ed5538904404c880</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;wirepulse::model::Entities&lt;Model&gt; &gt; &gt; wirepulse::db::query(bsoncxx::v_noabi::document::value, const wirepulse::model::EntitiesQuery&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::Manufacturer]</td>
<td align="right">639</td>
<td>/opt/wirepulse/inventory-api/src/http/global.hpp</td>
<td>wirepulse::http::Response wirepulse::http::global::retrieveAll(const spt::http2::framework::Request&amp;, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with Model = wirepulse::model::Manufacturer; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;)]</td>
<td align="right">386</td>
<td> </td>
<td align="right">58551917</td>
<td>2025-03-26 17:42:16.776</td>
</tr>
<tr>
<td>67e482c8ed5538904404c880</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;wirepulse::model::Entities&lt;Model&gt; &gt; &gt; wirepulse::db::query(bsoncxx::v_noabi::document::value, const wirepulse::model::EntitiesQuery&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::Manufacturer]</td>
<td align="right">663</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;wirepulse::model::Entities&lt;Model&gt; &gt; &gt; wirepulse::db::query(bsoncxx::v_noabi::document::value, const wirepulse::model::EntitiesQuery&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::Manufacturer]</td>
<td align="right">665</td>
<td> </td>
<td align="right">51710399</td>
<td>2025-03-26 17:42:16.776</td>
</tr>
<tr>
<td>67e482c8ed5538904404c880</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, int&gt; wirepulse::db::count(bsoncxx::v_noabi::document::view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::Manufacturer]</td>
<td align="right">50</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;wirepulse::model::Entities&lt;Model&gt; &gt; &gt; wirepulse::db::query(bsoncxx::v_noabi::document::value, const wirepulse::model::EntitiesQuery&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::Manufacturer]</td>
<td align="right">697</td>
<td> </td>
<td align="right">5138666</td>
<td>2025-03-26 17:42:16.828</td>
</tr>
<tr>
<td>67e482c8ed5538904404c880</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;bsoncxx::v_noabi::oid&gt; &gt; wirepulse::db::lastId(bsoncxx::v_noabi::document::view, bsoncxx::v_noabi::document::value&amp;&amp;, bsoncxx::v_noabi::document::value&amp;&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::Manufacturer]</td>
<td align="right">82</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;wirepulse::model::Entities&lt;Model&gt; &gt; &gt; wirepulse::db::query(bsoncxx::v_noabi::document::value, const wirepulse::model::EntitiesQuery&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::Manufacturer]</td>
<td align="right">705</td>
<td> </td>
<td align="right">1676136</td>
<td>2025-03-26 17:42:16.833</td>
</tr>
<tr>
<td>67e482c8ed5538904404c880</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/common.hpp</td>
<td>void wirepulse::http::output(const spt::http2::framework::Request&amp;, Response&amp;, const M&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::Entities&lt;wirepulse::model::Manufacturer&gt;]</td>
<td align="right">52</td>
<td>/opt/wirepulse/inventory-api/src/http/global.hpp</td>
<td>wirepulse::http::Response wirepulse::http::global::retrieveAll(const spt::http2::framework::Request&amp;, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with Model = wirepulse::model::Manufacturer; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;)]</td>
<td align="right">404</td>
<td> </td>
<td align="right">242595</td>
<td>2025-03-26 17:42:16.834</td>
</tr>
</tbody></table>

The table panel was set up in Grafana to display the child process records for the latest
APM record.  The query used to generate the data is show below.

<code-block lang="SQL">
<![CDATA[
with latest as 
(select id from webapm where type is null order by timestamp desc limit 1)
select webapm.id, type, file, function, line, caller_file, caller_function, caller_line, note, duration, timestamp
from webapm
inner join latest on webapm.id = latest.id
where type <> null
order by timestamp
]]>
</code-block>