# APM Sample Data
Sample data extracted from Grafana to illustrate some of the information captured from one of our REST
API services.

Image below shows a panel set up on our Grafana instance.  We set up some dynamic variables to allow
us to quickly filter the metrics being displayed as appropriate.

<img src="apm-grafana.png" alt="Grafana Panel" thumbnail="true"/>

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
and action in ($action)
and entity in ($entity)
and customer in ($customer)
and application in ($application)
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
(
  select id 
  from webapm 
  where type is null
  and action in ($action)
  and entity in ($entity)
  and customer in ($customer)
  and application in ($application)
  order by timestamp desc limit 1
)
select webapm.id, type, file, function, line, caller_file, caller_function, caller_line, note, duration, timestamp
from webapm
inner join latest on webapm.id = latest.id
where type <> null
order by timestamp
]]>
</code-block>

## All Records
The table below shows all APM records captured order by the `timestamp`.  I plan to develop
a simple viewer which will render this data as a *tree-table*, which will show the function
call sequence as a tree.

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
<th title="Field #10">error</th>
<th title="Field #11">duration</th>
<th title="Field #12">timestamp</th>
<th title="Field #13">application</th>
<th title="Field #14">entity</th>
<th title="Field #15">customer</th>
<th title="Field #16">host</th>
<th title="Field #17">request_method</th>
<th title="Field #18">response_status</th>
<th title="Field #19">city</th>
<th title="Field #20">country</th>
<th title="Field #21">request_path</th>
<th title="Field #22">username</th>
<th title="Field #23">role</th>
</tr></thead>
<tbody><tr>
<td>67ebbaad6d1350c70004a1a0</td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">13169294</td>
<td>2025-04-01 05:06:37</td>
<td>inventory-api</td>
<td>InventoryItem</td>
<td>bge</td>
<td>stage-v2-1</td>
<td>GET</td>
<td align="right">200</td>
<td>Vinnytsia</td>
<td>Ukraine</td>
<td>/inventory/item/id/6760bab01f42e4ffb10fe5b5</td>
<td>dmytro-bge</td>
<td>admin</td>
</tr>
<tr>
<td>67ebbaad6d1350c70004a1a0</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">76</td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">13155461</td>
<td>2025-04-01 05:06:37</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbaad6d1350c70004a1a0</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">411</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">77</td>
<td> </td>
<td> </td>
<td align="right">13135778</td>
<td>2025-04-01 05:06:37</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbaad6d1350c70004a1a0</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">87</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">417</td>
<td> </td>
<td> </td>
<td align="right">3144185</td>
<td>2025-04-01 05:06:37</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbaad6d1350c70004a1a0</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">65</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">107</td>
<td> </td>
<td> </td>
<td align="right">3139896</td>
<td>2025-04-01 05:06:37</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbaad6d1350c70004a1a0</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">159</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">72</td>
<td>Retrieve token</td>
<td> </td>
<td align="right">3136598</td>
<td>2025-04-01 05:06:37</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbaad6d1350c70004a1a0</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">101</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">171</td>
<td>Retrieve JWT token</td>
<td> </td>
<td align="right">3045546</td>
<td>2025-04-01 05:06:37</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbaad6d1350c70004a1a0</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">113</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">114</td>
<td> </td>
<td> </td>
<td align="right">3036940</td>
<td>2025-04-01 05:06:37</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbaad6d1350c70004a1a0</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">441</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">438</td>
<td> </td>
<td> </td>
<td align="right">9817445</td>
<td>2025-04-01 05:06:37</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbaad6d1350c70004a1a0</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">246</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">457</td>
<td> </td>
<td> </td>
<td align="right">9782498</td>
<td>2025-04-01 05:06:37</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbaad6d1350c70004a1a0</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">267</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">268</td>
<td> </td>
<td> </td>
<td align="right">3717640</td>
<td>2025-04-01 05:06:37</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbaad6d1350c70004a1a0</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/common.hpp</td>
<td>void wirepulse::http::output(const spt::http2::framework::Request&amp;, Response&amp;, const M&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">52</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">462</td>
<td> </td>
<td> </td>
<td align="right">135217</td>
<td>2025-04-01 05:06:37</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc6f6d1350c70004a1a1</td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">7668427</td>
<td>2025-04-01 05:14:07</td>
<td>inventory-api</td>
<td>InventoryItem</td>
<td>bge</td>
<td>stage-v2-1</td>
<td>GET</td>
<td align="right">200</td>
<td>Vinnytsia</td>
<td>Ukraine</td>
<td>/inventory/item/id/6760bab01f42e4ffb10fe5b5</td>
<td>dmytro-bge</td>
<td>admin</td>
</tr>
<tr>
<td>67ebbc6f6d1350c70004a1a1</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">76</td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">7656332</td>
<td>2025-04-01 05:14:07</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc6f6d1350c70004a1a1</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">411</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">77</td>
<td> </td>
<td> </td>
<td align="right">7640859</td>
<td>2025-04-01 05:14:07</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc6f6d1350c70004a1a1</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">87</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">417</td>
<td> </td>
<td> </td>
<td align="right">3728097</td>
<td>2025-04-01 05:14:07</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc6f6d1350c70004a1a1</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">65</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">107</td>
<td> </td>
<td> </td>
<td align="right">3722538</td>
<td>2025-04-01 05:14:07</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc6f6d1350c70004a1a1</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">159</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">72</td>
<td>Retrieve token</td>
<td> </td>
<td align="right">3720644</td>
<td>2025-04-01 05:14:07</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc6f6d1350c70004a1a1</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">101</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">171</td>
<td>Retrieve JWT token</td>
<td> </td>
<td align="right">3682458</td>
<td>2025-04-01 05:14:07</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc6f6d1350c70004a1a1</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">113</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">114</td>
<td> </td>
<td> </td>
<td align="right">3673773</td>
<td>2025-04-01 05:14:07</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc6f6d1350c70004a1a1</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">441</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">438</td>
<td> </td>
<td> </td>
<td align="right">3751604</td>
<td>2025-04-01 05:14:07</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc6f6d1350c70004a1a1</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">246</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">457</td>
<td> </td>
<td> </td>
<td align="right">3733821</td>
<td>2025-04-01 05:14:07</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc6f6d1350c70004a1a1</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">267</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">268</td>
<td> </td>
<td> </td>
<td align="right">3730173</td>
<td>2025-04-01 05:14:07</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc6f6d1350c70004a1a1</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/common.hpp</td>
<td>void wirepulse::http::output(const spt::http2::framework::Request&amp;, Response&amp;, const M&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">52</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">462</td>
<td> </td>
<td> </td>
<td align="right">138705</td>
<td>2025-04-01 05:14:07</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc916d1350c70004a1a2</td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">7224091</td>
<td>2025-04-01 05:14:41</td>
<td>inventory-api</td>
<td>InventoryItem</td>
<td>bge</td>
<td>stage-v2-1</td>
<td>GET</td>
<td align="right">200</td>
<td>Vinnytsia</td>
<td>Ukraine</td>
<td>/inventory/item/id/6760bab01f42e4ffb10fe5b5</td>
<td>dmytro-bge</td>
<td>admin</td>
</tr>
<tr>
<td>67ebbc916d1350c70004a1a2</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">76</td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">7211275</td>
<td>2025-04-01 05:14:41</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc916d1350c70004a1a2</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">411</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">77</td>
<td> </td>
<td> </td>
<td align="right">7195487</td>
<td>2025-04-01 05:14:41</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc916d1350c70004a1a2</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">87</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">417</td>
<td> </td>
<td> </td>
<td align="right">4308490</td>
<td>2025-04-01 05:14:41</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc916d1350c70004a1a2</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">65</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">107</td>
<td> </td>
<td> </td>
<td align="right">4303583</td>
<td>2025-04-01 05:14:41</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc916d1350c70004a1a2</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">159</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">72</td>
<td>Retrieve token</td>
<td> </td>
<td align="right">4301503</td>
<td>2025-04-01 05:14:41</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc916d1350c70004a1a2</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">101</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">171</td>
<td>Retrieve JWT token</td>
<td> </td>
<td align="right">4241796</td>
<td>2025-04-01 05:14:41</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc916d1350c70004a1a2</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">113</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">114</td>
<td> </td>
<td> </td>
<td align="right">4233326</td>
<td>2025-04-01 05:14:41</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc916d1350c70004a1a2</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">441</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">438</td>
<td> </td>
<td> </td>
<td align="right">2731669</td>
<td>2025-04-01 05:14:41</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc916d1350c70004a1a2</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">246</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">457</td>
<td> </td>
<td> </td>
<td align="right">2715618</td>
<td>2025-04-01 05:14:41</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc916d1350c70004a1a2</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">267</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">268</td>
<td> </td>
<td> </td>
<td align="right">2711653</td>
<td>2025-04-01 05:14:41</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc916d1350c70004a1a2</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/common.hpp</td>
<td>void wirepulse::http::output(const spt::http2::framework::Request&amp;, Response&amp;, const M&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">52</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">462</td>
<td> </td>
<td> </td>
<td align="right">134225</td>
<td>2025-04-01 05:14:41</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc936d1350c70004a1a3</td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">11671430</td>
<td>2025-04-01 05:14:43</td>
<td>inventory-api</td>
<td>InventoryItem</td>
<td>bge</td>
<td>stage-v2-1</td>
<td>GET</td>
<td align="right">200</td>
<td>Vinnytsia</td>
<td>Ukraine</td>
<td>/inventory/item/id/6760bab01f42e4ffb10fe5b5</td>
<td>dmytro-bge</td>
<td>admin</td>
</tr>
<tr>
<td>67ebbc936d1350c70004a1a3</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">76</td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">11662140</td>
<td>2025-04-01 05:14:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc936d1350c70004a1a3</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">411</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">77</td>
<td> </td>
<td> </td>
<td align="right">11635868</td>
<td>2025-04-01 05:14:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc936d1350c70004a1a3</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">87</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">417</td>
<td> </td>
<td> </td>
<td align="right">2256682</td>
<td>2025-04-01 05:14:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc936d1350c70004a1a3</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">65</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">107</td>
<td> </td>
<td> </td>
<td align="right">2251811</td>
<td>2025-04-01 05:14:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc936d1350c70004a1a3</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">159</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">72</td>
<td>Retrieve token</td>
<td> </td>
<td align="right">2249718</td>
<td>2025-04-01 05:14:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc936d1350c70004a1a3</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">101</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">171</td>
<td>Retrieve JWT token</td>
<td> </td>
<td align="right">2223012</td>
<td>2025-04-01 05:14:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc936d1350c70004a1a3</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">113</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">114</td>
<td> </td>
<td> </td>
<td align="right">2214158</td>
<td>2025-04-01 05:14:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc936d1350c70004a1a3</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">441</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">438</td>
<td> </td>
<td> </td>
<td align="right">9235224</td>
<td>2025-04-01 05:14:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc936d1350c70004a1a3</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">246</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">457</td>
<td> </td>
<td> </td>
<td align="right">9218791</td>
<td>2025-04-01 05:14:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc936d1350c70004a1a3</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">267</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">268</td>
<td> </td>
<td> </td>
<td align="right">9215017</td>
<td>2025-04-01 05:14:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbc936d1350c70004a1a3</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/common.hpp</td>
<td>void wirepulse::http::output(const spt::http2::framework::Request&amp;, Response&amp;, const M&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">52</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">462</td>
<td> </td>
<td> </td>
<td align="right">123731</td>
<td>2025-04-01 05:14:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbd466d1350c70004a1a4</td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">7236197</td>
<td>2025-04-01 05:17:42</td>
<td>inventory-api</td>
<td>InventoryItem</td>
<td>bge</td>
<td>stage-v2-1</td>
<td>GET</td>
<td align="right">200</td>
<td>Vinnytsia</td>
<td>Ukraine</td>
<td>/inventory/item/id/6760bab01f42e4ffb10fe5b5</td>
<td>dmytro-bge</td>
<td>admin</td>
</tr>
<tr>
<td>67ebbd466d1350c70004a1a4</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">76</td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">7222512</td>
<td>2025-04-01 05:17:42</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbd466d1350c70004a1a4</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">411</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">77</td>
<td> </td>
<td> </td>
<td align="right">7205483</td>
<td>2025-04-01 05:17:42</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbd466d1350c70004a1a4</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">87</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">417</td>
<td> </td>
<td> </td>
<td align="right">4419360</td>
<td>2025-04-01 05:17:42</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbd466d1350c70004a1a4</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">65</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">107</td>
<td> </td>
<td> </td>
<td align="right">4413415</td>
<td>2025-04-01 05:17:42</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbd466d1350c70004a1a4</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">159</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">72</td>
<td>Retrieve token</td>
<td> </td>
<td align="right">4411607</td>
<td>2025-04-01 05:17:42</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbd466d1350c70004a1a4</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">101</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">171</td>
<td>Retrieve JWT token</td>
<td> </td>
<td align="right">4349848</td>
<td>2025-04-01 05:17:42</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbd466d1350c70004a1a4</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">113</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">114</td>
<td> </td>
<td> </td>
<td align="right">4341644</td>
<td>2025-04-01 05:17:42</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbd466d1350c70004a1a4</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">441</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">438</td>
<td> </td>
<td> </td>
<td align="right">2633566</td>
<td>2025-04-01 05:17:42</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbd466d1350c70004a1a4</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">246</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">457</td>
<td> </td>
<td> </td>
<td align="right">2604841</td>
<td>2025-04-01 05:17:42</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbd466d1350c70004a1a4</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">267</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">268</td>
<td> </td>
<td> </td>
<td align="right">2601322</td>
<td>2025-04-01 05:17:42</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbd466d1350c70004a1a4</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/common.hpp</td>
<td>void wirepulse::http::output(const spt::http2::framework::Request&amp;, Response&amp;, const M&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">52</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">462</td>
<td> </td>
<td> </td>
<td align="right">131397</td>
<td>2025-04-01 05:17:42</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbe646d1350c70004a1a5</td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">5537821</td>
<td>2025-04-01 05:22:28</td>
<td>inventory-api</td>
<td>InventoryItem</td>
<td>bge</td>
<td>stage-v2-1</td>
<td>GET</td>
<td align="right">200</td>
<td>Vinnytsia</td>
<td>Ukraine</td>
<td>/inventory/item/id/6760bab01f42e4ffb10fe5b5</td>
<td>dmytro-bge</td>
<td>admin</td>
</tr>
<tr>
<td>67ebbe646d1350c70004a1a5</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">76</td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">5525221</td>
<td>2025-04-01 05:22:28</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbe646d1350c70004a1a5</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">411</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">77</td>
<td> </td>
<td> </td>
<td align="right">5509700</td>
<td>2025-04-01 05:22:28</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbe646d1350c70004a1a5</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">87</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">417</td>
<td> </td>
<td> </td>
<td align="right">2780209</td>
<td>2025-04-01 05:22:28</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbe646d1350c70004a1a5</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">65</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">107</td>
<td> </td>
<td> </td>
<td align="right">2766325</td>
<td>2025-04-01 05:22:28</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbe646d1350c70004a1a5</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">159</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">72</td>
<td>Retrieve token</td>
<td> </td>
<td align="right">2764309</td>
<td>2025-04-01 05:22:28</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbe646d1350c70004a1a5</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">101</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">171</td>
<td>Retrieve JWT token</td>
<td> </td>
<td align="right">2707541</td>
<td>2025-04-01 05:22:28</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbe646d1350c70004a1a5</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">113</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">114</td>
<td> </td>
<td> </td>
<td align="right">2698520</td>
<td>2025-04-01 05:22:28</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbe646d1350c70004a1a5</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">441</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">438</td>
<td> </td>
<td> </td>
<td align="right">2582278</td>
<td>2025-04-01 05:22:28</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbe646d1350c70004a1a5</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">246</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">457</td>
<td> </td>
<td> </td>
<td align="right">2567499</td>
<td>2025-04-01 05:22:28</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbe646d1350c70004a1a5</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">267</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">268</td>
<td> </td>
<td> </td>
<td align="right">2564425</td>
<td>2025-04-01 05:22:28</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbe646d1350c70004a1a5</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/common.hpp</td>
<td>void wirepulse::http::output(const spt::http2::framework::Request&amp;, Response&amp;, const M&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">52</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">462</td>
<td> </td>
<td> </td>
<td align="right">126399</td>
<td>2025-04-01 05:22:28</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbecf6d1350c70004a1a6</td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">5756228</td>
<td>2025-04-01 05:24:15</td>
<td>inventory-api</td>
<td>InventoryItem</td>
<td>bge</td>
<td>stage-v2-1</td>
<td>GET</td>
<td align="right">200</td>
<td>Vinnytsia</td>
<td>Ukraine</td>
<td>/inventory/item/id/6760bab01f42e4ffb10fe5b5</td>
<td>dmytro-bge</td>
<td>admin</td>
</tr>
<tr>
<td>67ebbecf6d1350c70004a1a6</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">76</td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">5732395</td>
<td>2025-04-01 05:24:15</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbecf6d1350c70004a1a6</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">411</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">77</td>
<td> </td>
<td> </td>
<td align="right">5714214</td>
<td>2025-04-01 05:24:15</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbecf6d1350c70004a1a6</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">87</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">417</td>
<td> </td>
<td> </td>
<td align="right">2858346</td>
<td>2025-04-01 05:24:15</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbecf6d1350c70004a1a6</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">65</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">107</td>
<td> </td>
<td> </td>
<td align="right">2852968</td>
<td>2025-04-01 05:24:15</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbecf6d1350c70004a1a6</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">159</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">72</td>
<td>Retrieve token</td>
<td> </td>
<td align="right">2851206</td>
<td>2025-04-01 05:24:15</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbecf6d1350c70004a1a6</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">101</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">171</td>
<td>Retrieve JWT token</td>
<td> </td>
<td align="right">2784375</td>
<td>2025-04-01 05:24:15</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbecf6d1350c70004a1a6</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">113</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">114</td>
<td> </td>
<td> </td>
<td align="right">2776050</td>
<td>2025-04-01 05:24:15</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbecf6d1350c70004a1a6</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">441</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">438</td>
<td> </td>
<td> </td>
<td align="right">2627135</td>
<td>2025-04-01 05:24:15</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbecf6d1350c70004a1a6</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">246</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">457</td>
<td> </td>
<td> </td>
<td align="right">2612550</td>
<td>2025-04-01 05:24:15</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbecf6d1350c70004a1a6</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">267</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">268</td>
<td> </td>
<td> </td>
<td align="right">2608882</td>
<td>2025-04-01 05:24:15</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbecf6d1350c70004a1a6</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/common.hpp</td>
<td>void wirepulse::http::output(const spt::http2::framework::Request&amp;, Response&amp;, const M&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">52</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">462</td>
<td> </td>
<td> </td>
<td align="right">205821</td>
<td>2025-04-01 05:24:15</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf276d1350c70004a1a7</td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">3752050</td>
<td>2025-04-01 05:25:43</td>
<td>inventory-api</td>
<td>InventoryItem</td>
<td>bge</td>
<td>stage-v2-1</td>
<td>GET</td>
<td align="right">200</td>
<td>Vinnytsia</td>
<td>Ukraine</td>
<td>/inventory/item/id/6760bab01f42e4ffb10fe5b5</td>
<td>dmytro-bge</td>
<td>admin</td>
</tr>
<tr>
<td>67ebbf276d1350c70004a1a7</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">76</td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">3739651</td>
<td>2025-04-01 05:25:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf276d1350c70004a1a7</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">411</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">77</td>
<td> </td>
<td> </td>
<td align="right">3725029</td>
<td>2025-04-01 05:25:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf276d1350c70004a1a7</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">87</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">417</td>
<td> </td>
<td> </td>
<td align="right">1661760</td>
<td>2025-04-01 05:25:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf276d1350c70004a1a7</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">65</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">107</td>
<td> </td>
<td> </td>
<td align="right">1644316</td>
<td>2025-04-01 05:25:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf276d1350c70004a1a7</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">159</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">72</td>
<td>Retrieve token</td>
<td> </td>
<td align="right">1642244</td>
<td>2025-04-01 05:25:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf276d1350c70004a1a7</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">101</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">171</td>
<td>Retrieve JWT token</td>
<td> </td>
<td align="right">1584359</td>
<td>2025-04-01 05:25:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf276d1350c70004a1a7</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">113</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">114</td>
<td> </td>
<td> </td>
<td align="right">1576136</td>
<td>2025-04-01 05:25:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf276d1350c70004a1a7</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">441</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">438</td>
<td> </td>
<td> </td>
<td align="right">1910152</td>
<td>2025-04-01 05:25:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf276d1350c70004a1a7</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">246</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">457</td>
<td> </td>
<td> </td>
<td align="right">1896094</td>
<td>2025-04-01 05:25:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf276d1350c70004a1a7</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">267</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">268</td>
<td> </td>
<td> </td>
<td align="right">1892361</td>
<td>2025-04-01 05:25:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf276d1350c70004a1a7</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/common.hpp</td>
<td>void wirepulse::http::output(const spt::http2::framework::Request&amp;, Response&amp;, const M&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">52</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">462</td>
<td> </td>
<td> </td>
<td align="right">122435</td>
<td>2025-04-01 05:25:43</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf516d1350c70004a1a8</td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">6290117</td>
<td>2025-04-01 05:26:25</td>
<td>inventory-api</td>
<td>InventoryItem</td>
<td>bge</td>
<td>stage-v2-1</td>
<td>GET</td>
<td align="right">200</td>
<td>Vinnytsia</td>
<td>Ukraine</td>
<td>/inventory/item/id/6760bab01f42e4ffb10fe5b5</td>
<td>dmytro-bge</td>
<td>admin</td>
</tr>
<tr>
<td>67ebbf516d1350c70004a1a8</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">76</td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">6280839</td>
<td>2025-04-01 05:26:25</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf516d1350c70004a1a8</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">411</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">77</td>
<td> </td>
<td> </td>
<td align="right">6273452</td>
<td>2025-04-01 05:26:25</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf516d1350c70004a1a8</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">87</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">417</td>
<td> </td>
<td> </td>
<td align="right">2041326</td>
<td>2025-04-01 05:26:25</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf516d1350c70004a1a8</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">65</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">107</td>
<td> </td>
<td> </td>
<td align="right">2037507</td>
<td>2025-04-01 05:26:25</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf516d1350c70004a1a8</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">159</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">72</td>
<td>Retrieve token</td>
<td> </td>
<td align="right">2035714</td>
<td>2025-04-01 05:26:25</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf516d1350c70004a1a8</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">101</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">171</td>
<td>Retrieve JWT token</td>
<td> </td>
<td align="right">2012592</td>
<td>2025-04-01 05:26:25</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf516d1350c70004a1a8</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">113</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">114</td>
<td> </td>
<td> </td>
<td align="right">2004439</td>
<td>2025-04-01 05:26:25</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf516d1350c70004a1a8</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">441</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">438</td>
<td> </td>
<td> </td>
<td align="right">4083513</td>
<td>2025-04-01 05:26:25</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf516d1350c70004a1a8</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">246</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">457</td>
<td> </td>
<td> </td>
<td align="right">4069766</td>
<td>2025-04-01 05:26:25</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf516d1350c70004a1a8</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">267</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">268</td>
<td> </td>
<td> </td>
<td align="right">4055932</td>
<td>2025-04-01 05:26:25</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebbf516d1350c70004a1a8</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/common.hpp</td>
<td>void wirepulse::http::output(const spt::http2::framework::Request&amp;, Response&amp;, const M&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">52</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">462</td>
<td> </td>
<td> </td>
<td align="right">130302</td>
<td>2025-04-01 05:26:25</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebc01e6d1350c70004a1a9</td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">4611147</td>
<td>2025-04-01 05:29:50</td>
<td>inventory-api</td>
<td>InventoryItem</td>
<td>bge</td>
<td>stage-v2-1</td>
<td>GET</td>
<td align="right">200</td>
<td>Vinnytsia</td>
<td>Ukraine</td>
<td>/inventory/item/id/6760bab01f42e4ffb10fe5b5</td>
<td>dmytro-bge</td>
<td>admin</td>
</tr>
<tr>
<td>67ebc01e6d1350c70004a1a9</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">76</td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td align="right">4601369</td>
<td>2025-04-01 05:29:50</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebc01e6d1350c70004a1a9</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">411</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/inventoryitem.cpp</td>
<td>wirepulse::http::addInventoryItemRoutes(spt::http2::framework::Server&lt;Response&gt;&amp;)::&lt;lambda(const spt::http2::framework::RoutingRequest&amp;, auto:72)&gt; [with auto:72 = boost::container::flat_map&lt;std::basic_string_view&lt;char&gt;, std::basic_string_view&lt;char&gt; &gt;]</td>
<td align="right">77</td>
<td> </td>
<td> </td>
<td align="right">4593941</td>
<td>2025-04-01 05:29:50</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebc01e6d1350c70004a1a9</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">87</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">417</td>
<td> </td>
<td> </td>
<td align="right">2331536</td>
<td>2025-04-01 05:29:50</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebc01e6d1350c70004a1a9</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">65</td>
<td>/opt/wirepulse/inventory-api/src/http/impl/common.cpp</td>
<td>wirepulse::http::AuthResponse wirepulse::http::authorise(const spt::http2::framework::Request&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">107</td>
<td> </td>
<td> </td>
<td align="right">2327311</td>
<td>2025-04-01 05:29:50</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebc01e6d1350c70004a1a9</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">159</td>
<td>/opt/wirepulse/inventory-api/src/db/storage.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::validateToken(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">72</td>
<td>Retrieve token</td>
<td> </td>
<td align="right">2325300</td>
<td>2025-04-01 05:29:50</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebc01e6d1350c70004a1a9</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">101</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>wirepulse::model::JwtToken::Ptr wirepulse::db::impl::token(std::string_view, spt::ilp::APMRecord&amp;)</td>
<td align="right">171</td>
<td>Retrieve JWT token</td>
<td> </td>
<td align="right">2300606</td>
<td>2025-04-01 05:29:50</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebc01e6d1350c70004a1a9</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">113</td>
<td>/opt/wirepulse/inventory-api/src/db/impl/tokenrepository.cpp</td>
<td>std::optional&lt;bsoncxx::v_noabi::oid&gt; {anonymous}::ptoken::jwtToken(const wirepulse::model::JwtToken&amp;, spt::ilp::APMRecord&amp;)</td>
<td align="right">114</td>
<td> </td>
<td> </td>
<td align="right">2292840</td>
<td>2025-04-01 05:29:50</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebc01e6d1350c70004a1a9</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">441</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">438</td>
<td> </td>
<td> </td>
<td align="right">2109647</td>
<td>2025-04-01 05:29:50</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebc01e6d1350c70004a1a9</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">246</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::retrieve(bsoncxx::v_noabi::oid, std::string_view, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem; std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">457</td>
<td> </td>
<td> </td>
<td align="right">2085656</td>
<td>2025-04-01 05:29:50</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebc01e6d1350c70004a1a9</td>
<td>step</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">267</td>
<td>/opt/wirepulse/inventory-api/src/db/repository.hpp</td>
<td>std::tuple&lt;short int, std::optional&lt;_Tp&gt; &gt; wirepulse::db::pipeline(std::vector&lt;spt::mongoservice::api::model::request::Pipeline::Document::Stage&gt;, spt::ilp::APMRecord&amp;, bool) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">268</td>
<td> </td>
<td> </td>
<td align="right">2081886</td>
<td>2025-04-01 05:29:50</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
<tr>
<td>67ebc01e6d1350c70004a1a9</td>
<td>function</td>
<td>/opt/wirepulse/inventory-api/src/http/common.hpp</td>
<td>void wirepulse::http::output(const spt::http2::framework::Request&amp;, Response&amp;, const M&amp;, spt::ilp::APMRecord&amp;) [with M = wirepulse::model::InventoryItem]</td>
<td align="right">52</td>
<td>/opt/wirepulse/inventory-api/src/http/template.hpp</td>
<td>wirepulse::http::Response wirepulse::http::get(const spt::http2::framework::Request&amp;, std::string_view, std::span&lt;const std::__cxx11::basic_string&lt;char&gt; &gt;, spt::ilp::APMRecord&amp;, AuthFunction&amp;&amp;) [with M = wirepulse::model::InventoryItem; AuthFunction = bool (*)(std::basic_string_view&lt;char&gt;, const wirepulse::model::JwtToken&amp;); std::string_view = std::basic_string_view&lt;char&gt;]</td>
<td align="right">462</td>
<td> </td>
<td> </td>
<td align="right">134830</td>
<td>2025-04-01 05:29:50</td>
<td>inventory-api</td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td align="right"></td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
<td> </td>
</tr>
</tbody></table>

## Post processed information
One of the reasons behind our decision to store the APM data generated by our services in
MongoDB before publishing to QuestDB was to post process the data.  In particular, we
send the client IP address through the [MMDB](mmdb.md) service to gather geographic
information about the client/visitor.

<img src="apm-by-country.png" alt="Visits by Country" thumbnail="true"/>