#!/Users/zzy/miniconda3/bin/python
"""Compile actual TeX and Espanso compositions, then run geometry regression oracles."""
from pathlib import Path
import argparse,json,subprocess,sys,tempfile,platform,shutil
import yaml
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'script'))
import geometry
HERE=Path(__file__).resolve().parent
PRELUDE='''#include <bits/stdc++.h>
using namespace std;
using ll=long long; using i128=__int128_t;
using DB=long double;
constexpr DB eps=1e-8L; const DB PI=acosl(-1);
constexpr int MAXN=10000;
#define all(a) (a).begin(),(a).end()
#define SZ(a) ((int)(a).size())
'''

def compiler(sanitize=True):
    if platform.system()!='Darwin':
        flags=[shutil.which('g++') or 'clang++','-std=c++17','-O1','-g']
    else:
        inc=Path('/opt/homebrew/opt/gcc/include/c++/15')
        arch=next(inc.glob('*-apple-darwin*'))
        sdk=subprocess.check_output(['xcrun','--show-sdk-path'],text=True).strip()
        flags=['/opt/homebrew/opt/llvm@22/bin/clang++','-std=c++17','-O1','-g',
               '-nostdinc++','-nostdlib++','-isystem',str(inc),'-isystem',str(arch),
               '-L/opt/homebrew/opt/gcc/lib/gcc/15','-Wl,-rpath,/opt/homebrew/opt/gcc/lib/gcc/15',
               '-lstdc++','-isysroot',sdk]
    if sanitize:
        flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
        if platform.system()=='Darwin':flags+=['-fno-sanitize=vptr,function']
    return flags

def compile_cpp(src,out,sanitize=True,extra=()):
    subprocess.run(compiler(sanitize)+list(extra)+[str(src),'-o',str(out)],check=True)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--espanso',type=Path,default=geometry.DEFAULT_ESPANSO)
    args=parser.parse_args()
    print('\n'.join(geometry.check(args.espanso)),flush=True)
    manifest,blocks=geometry.load()
    matches={m['trigger']:m['replace'] for m in yaml.safe_load(args.espanso.read_text())['matches']}
    with tempfile.TemporaryDirectory(prefix='geometry-tests-') as td:
        tmp=Path(td)
        # Every trigger is compiled with its documented dependencies, not with the whole library.
        for key in manifest['snippets']:
            seen=set();pieces=[]
            def append(k):
                if k in seen:return
                seen.add(k)
                for dep in manifest['dependencies'].get(k,[]):append(dep)
                pieces.append(matches[k])
            append(key)
            src=tmp/'closure.cpp';src.write_text(PRELUDE+'\n'.join(pieces)+'\nint main(){}\n')
            subprocess.run(compiler(False)[:]+['-fsyntax-only',str(src)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE)
        print('PASS 23 snippet dependency closures (C++17)',flush=True)
        lib='\n'.join(blocks[k] for k in manifest['order'])
        src=tmp/'driver.cpp';shutil.copyfile(HERE/'driver.cpp',src)
        # TeX and actual Espanso differ only in composition; check() enforces byte-for-byte equality.
        (tmp/'library.hpp').write_text(PRELUDE+lib)
        binary=tmp/'driver';compile_cpp(src,binary)
        subprocess.run([sys.executable,str(HERE/'test_geometry.py'),str(binary)],check=True)
        # Compile and exercise both supported floating types with LOCAL assertions enabled.
        for db in ['double','long double']:
            (tmp/'library.hpp').write_text(PRELUDE.replace('using DB=long double;',f'using DB={db};')+lib)
            compile_cpp(src,binary,extra=['-DLOCAL'])
            subprocess.run([str(binary)],input='fixed\n',text=True,check=True)
        print('PASS LOCAL fixed cases for DB=double and long double',flush=True)
if __name__=='__main__':main()
