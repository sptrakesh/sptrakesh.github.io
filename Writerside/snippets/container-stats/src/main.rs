mod ilp;
mod stats;

use clap::Parser;
use std::process::Command;
use std::io::{BufRead, BufReader};

use ilp::publish;
use stats::{RawStats, createStats};
use stats::Stats;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Cli {
  #[arg(short, long, default_value = "containerStats")]
  table: String,
  #[arg(short = 'n', long = "node")]
  host: String,
  #[arg(short, long, default_value = "localhost")]
  questdb: String
}

fn main()
{
  let args = Cli::parse();
  
  let output = Command::new("docker").arg("stats").arg("--no-stream").arg("--format=json").output().expect("failed to execute process");
  let mut reader = BufReader::new(output.stdout.as_slice());
  let mut vec : Vec<Stats> = Vec::new();
  let mut line = String::new();
  while reader.read_line(&mut line).unwrap() > 0
  {
    let raw : RawStats = serde_json::from_str(&line.trim()).expect("failed to parse json");
    let stats = createStats(&raw);
    if cfg!(target_os = "macos") { println!("{:?}", stats); }
    vec.push(stats);
    line.clear();
  }
  
  if vec.len() > 0 { publish(&args, &vec).expect("Failed to publish stats"); }
}
