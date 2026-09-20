#!/Users/zzy/miniconda3/bin/python
"""Extract marked TeX code, check Espanso, or explicitly synchronize its snippets."""
from pathlib import Path
import argparse,json,re,sys
import yaml
ROOT=Path(__file__).resolve().parents[1]
DEFAULT_ESPANSO=Path.home()/'Library/Application Support/espanso/match/computational_geometry.yml'

class Flow(list):
    """List dumped inline, matching the hand-written `[a, b]` style of the other Espanso files."""

def load(root=ROOT):
    manifest=json.loads((root/'script/geometry_snippets.json').read_text())
    tex=(root/manifest['source']).read_text()
    pairs=re.findall(r'% geometry-code: ([\w-]+)\s*\\begin\{minted\}\{cpp\}\n(.*?)\\end\{minted\}',tex,re.S)
    blocks={name:code.strip()+'\n' for name,code in pairs}
    assert len(pairs)==len(blocks),'duplicate geometry code marker'
    assert set(blocks)==set(manifest['order']),'manifest/code block mismatch'
    return manifest,blocks

def primary(match):
    # Espanso allows either a single `trigger` or a `triggers` list; the first one is canonical here.
    return match['trigger'] if 'trigger' in match else match['triggers'][0]

def snippets(root=ROOT):
    manifest,blocks=load(root)
    return {trigger:'\n\n'.join(blocks[name].rstrip() for name in names)+'\n'
            for trigger,names in manifest['snippets'].items()}

def check(path=DEFAULT_ESPANSO,root=ROOT):
    manifest,_=load(root);expected=snippets(root)
    matches=yaml.safe_load(path.read_text())['matches']
    actual={primary(m):m for m in matches}
    assert len(actual)==len(matches),'duplicate trigger'
    assert actual.keys()==expected.keys(),'trigger set mismatch'
    bad=[key for key in expected if expected[key]!=actual[key]['replace']]
    assert not bad,'TeX/Espanso differ: '+', '.join(bad)
    bad=[key for key,alias in manifest['aliases'].items() if actual[key].get('triggers')!=[key]+alias]
    assert not bad,'alias triggers differ: '+', '.join(bad)
    bad=[key for key,terms in manifest['search_terms'].items() if actual[key].get('search_terms')!=terms]
    assert not bad,'search_terms differ: '+', '.join(bad)
    return [f'{len(actual)} Espanso snippets == marked TeX blocks ✓',
            f"{sum(len(v) for v in manifest['aliases'].values())} alias triggers, "
            f"{sum(len(v) for v in manifest['search_terms'].values())} search terms ✓"]

def code_size(s):
    # C++ strings/character literals are preserved; comments and whitespace excluded.
    s=re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*.*?\*/',
             lambda m:'' if m[0].startswith(('//','/*')) else m[0],s,flags=re.S)
    return sum(bool(line.strip()) for line in s.splitlines()),len(re.sub(r'\s','',s))

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--espanso',type=Path,default=DEFAULT_ESPANSO)
    parser.add_argument('--write-espanso',action='store_true',help='explicitly overwrite only geometry replace fields')
    parser.add_argument('--stats',action='store_true')
    args=parser.parse_args()
    if args.write_espanso:
        cfg=yaml.safe_load(args.espanso.read_text());manifest,_=load();expected=snippets()
        assert {primary(m) for m in cfg['matches']}==expected.keys()
        for i,m in enumerate(cfg['matches']):
            key=primary(m);alias=manifest['aliases'].get(key);terms=manifest['search_terms'].get(key)
            head={'triggers':Flow([key]+alias)} if alias else {'trigger':key}
            rest={k:v for k,v in m.items() if k not in ('trigger','triggers','label','search_terms','replace')}
            cfg['matches'][i]={**head,'label':m['label'],
                               **({'search_terms':Flow(terms)} if terms else {}),**rest,'replace':expected[key]}
        class Dumper(yaml.SafeDumper):pass
        Dumper.add_representer(str,lambda d,s:d.represent_scalar('tag:yaml.org,2002:str',s,style='|' if '\n' in s else None))
        Dumper.add_representer(Flow,lambda d,s:d.represent_sequence('tag:yaml.org,2002:seq',s,flow_style=True))
        args.espanso.write_text('# Generated from icpc-ccpc-template sections/13_geometry.tex.\n'
                               '# Sync/check: script/geometry.py (see template-check/geometry/README.md).\n'
                               +yaml.dump(cfg,Dumper=Dumper,allow_unicode=True,sort_keys=False,width=100))
    print('\n'.join(check(args.espanso)))
    if args.stats:
        before={m['trigger']:m['replace'] for m in yaml.safe_load((ROOT/'archive/espanso_geometry_20260920_before_slim.yml.txt').read_text())['matches']}
        for key,value in snippets().items():print(key,code_size(before[key]),'->',code_size(value))
if __name__=='__main__':main()
