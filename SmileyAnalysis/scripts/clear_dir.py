"""
Clears directory with data from SmileyAnalysis
"""
import argparse
import datetime
from enum import Enum
from pathlib import Path
import sys

class FileType(Enum):
  LOGFILE = 0
  CATEVNTFILE = 1
  ROOTFILE = 2
  PBSLOGFILES = 3

def get_file_type(ftstr):
  file_types = {x.name:x for x in FileType}
  if ftstr not in file_types:
    print("Unknown file type.")
    sys.exit(1)
  return file_types[ftstr]

def get_file_patterns(file_type):
  patterns = {
    FileType.LOGFILE : ["*.log"],
    FileType.CATEVNTFILE : ["*.cat.evt.root"],
    FileType.ROOTFILE : ["*.root"],
    FileType.PBSLOGFILES : ["*.o*-*","*.e*-*"]
  }
  return patterns[file_type]

def remove_files(work_dir_path,file_type):
  start_date = datetime.datetime.now()
  print("Start removing files ...")
  removed_files_counter = 0
  for fp in get_file_patterns(file_type):
    files = list(work_dir_path.glob(fp))
    n_files = len(files)
    print("Found files to remove :",n_files)
    for i,file in enumerate(files):
      file.unlink()
      print(i+1,"/",n_files)
      removed_files_counter += 1
  elapsed_time = datetime.datetime.now() - start_date
  print("Removed files :",removed_files_counter)
  print("Elapsed time  :",elapsed_time)

ap = argparse.ArgumentParser()
ap.add_argument(
    "--dir",
    type=str,
    required=True,
    help="Work directory"
)
ap.add_argument(
    "--file-type",
    type=str,
    required=True,
    help="File type: "+"".join([x.name + "," for x in FileType])
)
args = ap.parse_args()
remove_files(Path(args.dir),get_file_type(args.file_type))



