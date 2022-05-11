"""
Extract histograms for SmileyAnalaysis
"""
import LHCbStyle as lhcb
import ROOT
from pathlib import Path
import argparse
import sys
import copy
import json

def is_efficiency_obj(name):
  return name[0:3] == "eff"
  
def is_1dim_hist(h):
  return h.GetDimension() == 1
  
def load_objects(file_path):
  objects = {"h1d":[],"h2d":[],"eff":[]}
  file = ROOT.TFile.Open(str(file_path),"READ")
  keys = file.GetListOfKeys()
  for key in keys:
    obj_name = key.GetName()
    obj = file.Get(obj_name)
    if is_efficiency_obj(obj_name):
      objects["eff"].append(copy.deepcopy(obj))
    elif is_1dim_hist(obj):
      objects["h1d"].append(copy.deepcopy(obj))
    else:
      objects["h2d"].append(copy.deepcopy(obj))
  file.Close()
  return objects

def get_img_path(output_dir_path,obj):
  img_name = obj.GetName() + ".png"
  return str(output_dir_path/img_name)

def format_obj(obj):
  obj.Scale(obj.GetEntries())
  obj.SetTitle("")
  obj.GetYaxis().SetTitleFont(132)
  obj.GetXaxis().SetTitleFont(132)

def draw_objects(output_dir_path,objects):
  ROOT.TGaxis.SetMaxDigits(3)
  c = ROOT.TCanvas("c","",800,800)
  for eff in objects["eff"]:
    eff.SetTitle("")
    eff.Draw()
    c.Print(get_img_path(output_dir_path,eff))
  for h in objects["h1d"]:
    format_obj(h)
    h.Draw("hist")
    c.Print(get_img_path(output_dir_path,h))
  for h in objects["h2d"]:
    format_obj(h)
    h.Draw("colz")
    c.Print(get_img_path(output_dir_path,h))

def is_deltaphi_hist(hist):
  return "DeltaPhi" in hist.GetName()

def extract_statistics(objects,output_dir_path):
  stats = {}
  for h in objects["h1d"]:
    if h.GetName() == "CHMN_FoundHitsSize":
      stats[u"events-detected"] = h.GetEntries()
    if "DeltaPhi" not in h.GetName():
      continue
    h.ResetStats()
    stats[u"{hn}".format(hn=h.GetName())] = {
      u"mean" : h.GetMean(1),
      u"mean-error" : h.GetMean(11),
      u"std" : h.GetStdDev(1),
      u"std-error" : h.GetStdDev(11),
      u"skewness" : h.GetSkewness(1),
      u"skewness-error" : h.GetSkewness(11),
      u"entries" : h.GetEntries()
    }
  file_path = output_dir_path/"statistics.json"
  with file_path.open(mode="w",encoding="utf-8") as file:
    file.write(u"{x}".format(x=json.dumps(stats, indent=2,ensure_ascii=False).encode('utf-8')))
  

def parse_args():
  ap = argparse.ArgumentParser()
  ap.add_argument(
      "--input-file",
      type=str,
      required=True,
      help="Path to input file"
  )
  ap.add_argument(
      "--output-dir",
      type=str,
      required=True,
      help="Path to output directory"
  )
  return ap.parse_args()

def main():
  args = parse_args()
  input_file_path = Path(args.input_file)
  output_dir_path = Path(args.output_dir)
  if not input_file_path.exists():
    print("Input file does not exist.")
    sys.exit(1)
  if not output_dir_path.exists():
    print("Creating output directory")
    output_dir_path.mkdir()
  objs = load_objects(input_file_path)
  draw_objects(output_dir_path,objs)
  extract_statistics(objs,output_dir_path)

if __name__ == "__main__":
  main()
