import argparse
from pathlib import Path
import sys

def parse_args():
  ap = argparse.ArgumentParser()
  ap.add_argument(
      "--input-dir",
      type=str,
      required=True,
      help="Path to the directory with *.unk.evt.root files"
  )
  ap.add_argument(
      "--output-dir",
      type=str,
      required=True,
      help="Path to the directory with *.unk.evt.root files"
  )
  ap.add_argument(
      "--sets-number",
      type=int,
      required=True,
      help="How many sets"
  )
  ap.add_argument(
      "--set-size",
      type=int,
      required=True,
      help="How many elements in each set (maximal number)"
  )
  return ap.parse_args()

def verify_params(input_dir_path,output_dir_path,sets_number,set_size):
  conditions = [
    [input_dir_path.exists() and input_dir_path.is_dir(),f"[ERROR] Input directory does not exist. Directory path: {input_dir_path}"],
    [output_dir_path.exists() and output_dir_path.is_dir(),f"[ERROR] Output directory does not exist. Directory path: {output_dir_path}"],
    [sets_number >= 1,f"[ERROR] Incorrect sets number: {sets_number}."],
    [set_size >= 1,f"[ERROR] Incorrect set size: {set_size}."]
  ]
  passed = True
  for c in conditions:
    if not c[0]:
      print(c[1])
      passed = False
  return passed

def generate_maps(input_dir_path,output_dir_path,sets_number,set_size):
  files = list(input_dir_path.glob("*.unk.evt.root"))
  n_files = len(files)
  if n_files == 0:
    print("[ERROR] Empty input directory. No *.unk.evt.root files.")
    sys.exit(1)
  max_index = min(sets_number*set_size,n_files) + 1
  for i,index_from in enumerate(list(range(0,max_index,set_size))):
    index_to = index_from + set_size
    if index_to > max_index:
      index_to = max_index
    output_file_path = output_dir_path/f"set_{i+1}.txt"
    files_per_set = files[index_from:index_to]
    with output_file_path.open(mode="w") as ofile:
      for ifile_path in files_per_set:
        ofile.write(f"{ifile_path}\n")


def main():
  args = parse_args()
  input_dir_path = Path(args.input_dir)
  output_dir_path = Path(args.output_dir)
  sets_number = args.sets_number
  set_size = args.set_size
  if not verify_params(input_dir_path,output_dir_path,sets_number,set_size):
    print("[ERROR] Incorrect parameters.")
    sys.exit(1)
  generate_maps(input_dir_path,output_dir_path,sets_number,set_size)

if __name__ == "__main__":
  main()