"""
Script for hadding files - it hadd only histograms
"""
import argparse
import copy
import datetime
from pathlib import Path
import ROOT

def is_efficiency_obj(name):
  return name[0:3] == "eff"

def build_data_structure(file):
  histograms = {}
  efficiencies = {}
  file_dir = file.Get("EventCategorizer subtask 0 stats")
  keys = file_dir.GetListOfKeys()
  for k in keys:
    kname = k.GetName()
    if is_efficiency_obj(kname):
      efficiencies[kname] = copy.deepcopy(file_dir.Get(kname))
    else:
      histograms[kname] = copy.deepcopy(file_dir.Get(kname))
  return histograms, efficiencies

def add_objects(histograms, efficiencies,file):
  file_dir = file.Get("EventCategorizer subtask 0 stats")
  for hname, hist in histograms.items():
    hist.Add(hist,file_dir.Get(hname))
  for ename, eff in efficiencies.items():
    eff.Add(file_dir.Get(ename))

def merge_files(input_dir_path):
  d1 = datetime.datetime.now()
  print("Merging files ...")
  files = list(input_dir_path.glob("*.cat.evt.root"))
  file = ROOT.TFile.Open(str(files[0]),"READ")
  histograms, efficiencies = build_data_structure(file)
  file.Close()
  n = len(files)
  errors_counter = 0
  for i in range(1,n):
    try :
      file_path = files[i]
      progress = str(i+1) + "/" + str(n)
      print(progress)
      file = ROOT.TFile.Open(str(file_path),"READ")
      add_objects(histograms, efficiencies,file)
      file.Close()
    except Exception:
      errors_counter += 1
      errors_msg = "Error counter : " + str(errors_counter)
      print(errors_msg)
  d2 = datetime.datetime.now()
  print(d2-d1)
  errors_msg = "Final error counter : " + str(errors_counter)
  print(errors_msg)
  return histograms, efficiencies

def save_results(histograms, efficiencies,output_dir_path):
  print("Saving results ...")
  output_file_path = output_dir_path/"results.root"
  print(output_file_path)
  file = ROOT.TFile.Open(str(output_file_path),"RECREATE")
  file.cd()
  for hname, hist in histograms.items():
    hist.Write(hname);
  for ename, eff in efficiencies.items():
    eff.Write(ename)
  file.Close()

ap = argparse.ArgumentParser()
ap.add_argument(
    "--input-dir",
    type=str,
    required=True,
    help="Input directory"
)
ap.add_argument(
    "--output-dir",
    type=str,
    required=True,
    help="Output directory"
)
args = ap.parse_args()
input_dir_path = Path(args.input_dir)
output_dir_path = Path(args.output_dir)
histograms, efficiencies = merge_files(input_dir_path)
save_results(histograms, efficiencies,output_dir_path)
