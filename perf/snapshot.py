#!/usr/bin/env python

import argparse
import os
import shutil
import subprocess


parser = argparse.ArgumentParser(description='Create a snapshot of .json files from Google Benchmark.')
parser.add_argument('--output-root', '-o', type=str, required=True,
                    help='the root of the tree into which to write the json files')
args = parser.parse_args()


git_log = subprocess.check_output(['git', 'log', '-1'])
commit = git_log.splitlines()[0][7:]

out_dir = os.path.abspath(args.output_root)

latest_dir = os.path.join(out_dir, 'latest_snapshot')
out_dir = os.path.join(out_dir, 'snapshots', commit)
if not os.path.exists(out_dir):
    os.mkdir(out_dir)

def copy_file(filename):
    shutil.copyfile(filename, os.path.join(latest_dir, filename))
    shutil.copyfile(filename, os.path.join(out_dir, filename))

copy_file('ctor_dtor_perf.json')
copy_file('copy_perf.json')
copy_file('insert_erase_perf.json')
copy_file('for_find_perf.json')
copy_file('compare_boyer_moore_perf.json')
copy_file('grapheme_prop_lookup_perf.json')
copy_file('normalize_perf.json')
copy_file('collation_element_lookup_perf_000.json')
copy_file('collation_element_lookup_perf_001.json')
copy_file('collation_element_lookup_perf_002.json')
copy_file('collation_element_lookup_perf_003.json')
copy_file('collation_element_lookup_perf_004.json')
copy_file('collation_element_lookup_perf_005.json')
copy_file('collation_element_lookup_perf_006.json')
copy_file('collation_perf_000.json')
copy_file('collation_perf_001.json')
copy_file('collation_perf_002.json')
copy_file('collation_perf_003.json')
copy_file('collation_perf_004.json')
copy_file('collation_perf_005.json')
copy_file('collation_perf_006.json')
