# APM Desktop Viewer

A simple [desktop](https://github.com/sptrakesh/apm-viewer/tree/main/desktop)
application built using [Qt](https://qt.io/) for viewing [APM](Application-Performance-Management.md)
data in a more natural hierarchical manner.

The raw APM data from [QuestDB](https://questdb.com) returns the APM data
in a flat tabular form.  This format does not reflect the hierarchical function
call chain when responding to a **REST API** request. This application parses the flat 
CSV data and renders a *forrest* of *trees*, where each *tree* represents the
function call chain for a specific **REST API** request.

## Strategy
There are a couple of different ways in which we can convert the flat CSV data
into a hierarchical model.
* Use the `caller_function` property to determine the caller of the current APM Process
  line, and build the hierarchy based on it.
* Compute the end time from the standard `timestamp` and `duration` properties.  Any
  function call that ends before a previously parsed process record is a child of that
  record.  I have used this strategy to build the hierarchical tree model.

<code-block lang="C++" collapsible="true">
<![CDATA[
Node* Node::appendChild( const Node& node )
{
  const auto et = node.timestamp + std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::nanoseconds( node.duration ) );

  for ( const auto& child : children )
  {
    const auto cet = child->timestamp + std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::nanoseconds( child->duration ) );
    if ( et >= cet ) continue;
    return child->appendChild( node );
  }

  auto n = appendChild();
  n->id = node.id;
  n->duration = node.duration;
  n->timestamp = node.timestamp;
  n->file = node.file;
  n->line = node.line;
  n->function = node.function;
  return n;
}
]]>
</code-block>

## CSV Data
CSV data can be exported from [QuestDB](https://questdb.com/).  I used our Grafana dashboard
that displays APM data to export the necessary APM data as CSV.  The following example
query is how I exported the latest APM data to CSV.

<code-block lang="SQL" collapsible="true">
with latest as 
(
  select id as apm_id
  from webapm 
  where type is null
  and action in ($action)
  and entity in ($entity)
  and application in ($application)
  order by timestamp desc
  limit $limit
)
select webapm.*, to_str(timestamp, 'yyyy-MM-ddTHH:mm:ss.U+') as 'timestamp_iso'
from webapm
inner join latest on webapm.id = latest.apm_id
order by timestamp
</code-block>

We have to use `with` to retrieve the `id`s for the root APM records (each record
corresponds to a REST API endpoint), and then retrieve all the APM process records
for those root records.  We need to use `with` to generate the list of `id`s, since
QuestDB does not support `in` query with a *sub-select*.

We also add an extra `timestamp_iso` column in the output which gives the `timestamp`
with micro-second precision.  The standard `timestamp` is stored in QuestDB with
micro-second precision, but standard SQL export only provides second level precision.
We need the full precision to build the call chain hierarchy properly.

## User Interface
The interface is extremely simple.  The window displays a [QTreeView](https://doc.qt.io/qt-6/qtreeview.html)
and [QTableView](https://doc.qt.io/qt-6/qtableview.html) to render the APM data.
The tree view displays the function call chain when responding to **REST API** endpoints.
The table view displays additional APM data that was collected for each *process*.

<img src="apm-desktop.png" alt="Desktop Application" thumbnail="true"/>