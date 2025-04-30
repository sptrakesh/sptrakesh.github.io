use questdb::{
  Result,
  ingress::{
    Sender,
    Buffer,
    TimestampNanos
  },
};
use chrono::Utc;

use super::Cli;
use super::stats::{Stats, Measurement};

pub fn publish(cli: &Cli, stats: &Vec<Stats>) -> Result<()>
{
  let now = Utc::now();
  let uri = {
    if cfg!(target_os = "macos") { "tcp::addr=localhost:9009".to_string() }
    else { format!("tcp::addr={}:9009", cli.questdb) }
  };

  let mut sender = Sender::from_conf(uri)?;
  let mut buffer = Buffer::new();
  
  fn add_io(buffer: &mut Buffer, measurement: &Measurement, prefix: &str)
  {
    if measurement.unit == "B" { buffer.column_f64(prefix, measurement.value as f64).expect("Failed to add IO B"); }
    else if measurement.unit == "KB" { buffer.column_f64(prefix, (measurement.value * 1024.0)  as f64).expect("Failed to add IO KB"); }
    else if measurement.unit == "MB" { buffer.column_f64(prefix, (measurement.value * 1024.0 * 1024.0)  as f64).expect("Failed to add IO MB"); }
    else if measurement.unit == "GB" { buffer.column_f64(prefix, (measurement.value * 1024.0 * 1024.0 * 1024.0)  as f64).expect("Failed to add IO GB"); }
    buffer.column_str(format!("{}_unit", prefix).as_str(), "bytes").expect("Failed to add IO unit");
  }
  
  fn add_memory(buffer: &mut Buffer, measurement: &Measurement, prefix: &str)
  {
    if measurement.unit == "B" { buffer.column_f64(prefix, measurement.value as f64).expect("Failed to add memory B"); }
    if measurement.unit == "KiB" { buffer.column_f64(prefix, (measurement.value * 1024.0) as f64).expect("Failed to add memory KiB"); }
    if measurement.unit == "MiB" { buffer.column_f64(prefix, (measurement.value * 1024.0 * 1024.0) as f64).expect("Failed to add memory MiB"); }
    if measurement.unit == "GiB" { buffer.column_f64(prefix, (measurement.value * 1024.0 * 1024.0 * 1024.0) as f64).expect("Failed to add memory GiB"); }
    buffer.column_str(format!("{}_unit", prefix).as_str(), "bytes").expect("Failed to add memory unit");
  }
  
  for stat in stats 
  {
    buffer.table(cli.table.as_str())?.
        symbol("host", cli.host.clone())?.
        symbol("container", stat.container.clone())?.
        symbol("name", stat.name.clone())?.
        column_str("id", stat.id.clone())?.
        column_f64("cpu", stat.cpuPercentage as f64)?.
        column_f64("memory_percentage", stat.memoryPercentage as f64)?.
        column_i64("pids", stat.pids as i64)?;
    
    add_io(&mut buffer, &stat.blockIO, "block_io");
    add_io(&mut buffer, &stat.netIO, "net_io");
    add_memory(&mut buffer, &stat.memoryUsage, "memory_use");
    add_memory(&mut buffer, &stat.totalMemory, "total_memory");
    
    buffer.at(TimestampNanos::from_datetime(now)?)?;
  }

  sender.flush(&mut buffer)?;
  Ok(())
}
