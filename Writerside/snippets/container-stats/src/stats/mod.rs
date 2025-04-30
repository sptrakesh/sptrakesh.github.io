use substring::Substring;
use serde::Deserialize;

#[derive(Deserialize, Debug)]
#[allow(non_snake_case)]
pub struct RawStats
{
  pub BlockIO: String,
  pub CPUPerc: String,
  pub Container: String,
  pub ID: String,
  pub MemPerc: String,
  pub MemUsage: String,
  pub Name: String,
  pub NetIO: String,
  pub PIDs: String
}

#[derive(Deserialize, Debug)]
#[allow(non_snake_case)]
pub struct Measurement
{
  pub value: f32,
  pub unit: String
}

#[derive(Deserialize, Debug)]
#[allow(non_snake_case)]
pub struct Stats
{
  pub id: String,
  pub container: String,
  pub name: String,
  pub blockIO: Measurement,
  pub cpuPercentage: f32,
  pub memoryPercentage: f32,
  pub memoryUsage: Measurement,
  pub totalMemory: Measurement,
  pub netIO: Measurement,
  pub pids: u32,
}

#[allow(non_snake_case)]
pub fn createStats(rs: &RawStats) -> Stats
{
  fn parseBlockIO(value: &str) -> Measurement
  {
    if value.ends_with("GB") || value.ends_with("gB")
    {
      let v = value.substring(0, value.len() - 2);
      return Measurement{value: v.parse::<f32>().expect("Failed to parse BlockIO"), unit: "GB".to_string()};
    }
    
    if value.ends_with("MB") || value.ends_with("mB")
    {
      let v = value.substring(0, value.len() - 2);
      return Measurement{value: v.parse::<f32>().expect("Failed to parse BlockIO"), unit: "MB".to_string()};
    }

    if value.ends_with("KB") || value.ends_with("kB")
    {
      let v = value.substring(0, value.len() - 2);
      return Measurement{value: v.parse::<f32>().expect("Failed to parse BlockIO"), unit: "KB".to_string()};
    }

    if value.ends_with("B")
    {
      let v = value.substring(0, value.len() - 1);
      return Measurement{value: v.parse::<f32>().expect("Failed to parse NetIO"), unit: "B".to_string()};
    }

    Measurement{value: 0.0, unit: String::new()}
  }
  
  fn parseMemory(value: &str) -> Measurement
  {
    let v = value.substring(0, value.len() - 3);
    Measurement{ value: v.parse::<f32>().expect("Failed to parse MemUsage"), unit: value.substring(v.len(), value.len()).to_string() }
  }

  fn parseNetIO(value: &str) -> Measurement
  {
    if value.ends_with("gB") || value.ends_with("GB")
    {
      let v = value.substring(0, value.len() - 2);
      return Measurement{value: v.parse::<f32>().expect("Failed to parse NetIO"), unit: "GB".to_string()};
    }

    if value.ends_with("mB") || value.ends_with("MB")
    {
      let v = value.substring(0, value.len() - 2);
      return Measurement{value: v.parse::<f32>().expect("Failed to parse NetIO"), unit: "MB".to_string()};
    }

    if value.ends_with("kB") || value.ends_with("KB")
    {
      let v = value.substring(0, value.len() - 2);
      return Measurement{value: v.parse::<f32>().expect("Failed to parse NetIO"), unit: "KB".to_string()};
    }
    
    if value.ends_with("B")
    {
      let v = value.substring(0, value.len() - 2);
      return Measurement{value: v.parse::<f32>().expect("Failed to parse NetIO"), unit: "B".to_string()};
    }
    
    Measurement{value: 0.0, unit: String::new()}
  }
  
  let mut stats = Stats{ id: rs.ID.clone(), container: rs.Container.clone(), name: rs.Name.clone(), 
    blockIO: Measurement{value: 0.0, unit:String::new()}, cpuPercentage: 0.0, memoryPercentage: 0.0, 
    memoryUsage: Measurement{value: 0.0, unit: String::new()}, 
    totalMemory: Measurement{value:0.0, unit: String::new()}, 
    netIO: Measurement{value: 0.0, unit: String::new()}, pids: 0};

  stats.cpuPercentage = rs.CPUPerc.replace("%", "").parse::<f32>().expect("Failed to parse CPUPerc");
  stats.memoryPercentage = rs.MemPerc.replace("%", "").parse::<f32>().expect("Failed to parse CPUPerc");
  stats.pids = rs.PIDs.parse::<u32>().expect("Failed to parse PIDs");
  
  let parts : Vec<&str> = rs.BlockIO.split(" / ").collect();
  if parts.len() > 0 { stats.blockIO = parseBlockIO(parts[0]); }
  
  let parts : Vec<&str> = rs.MemUsage.split(" / ").collect();
  if parts.len() > 0
  {
    let mem = parseMemory(parts[0]);
    stats.memoryUsage.value = mem.value;
    stats.memoryUsage.unit = mem.unit.clone();
  }
  if parts.len() > 1
  {
    let mem = parseMemory(parts[1]);
    stats.totalMemory.value = mem.value;
    stats.totalMemory.unit = mem.unit.clone();
  }

  let parts : Vec<&str> = rs.NetIO.split(" / ").collect();
  if parts.len() > 0 { stats.netIO = parseNetIO(parts[0]); }
  
  stats
}