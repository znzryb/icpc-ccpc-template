#!/Users/zzy/miniconda3/bin/python
"""Independent Shapely/GEOS oracles and exact integer reference properties."""
from pathlib import Path
import subprocess,random,math,itertools,json,sys
from shapely.geometry import MultiPoint,Point,Polygon,LineString
from shapely import minimum_bounding_radius
D=Path(__file__).resolve().parent
BIN=Path(sys.argv[1]) if len(sys.argv)>1 else D/'driver'
rng=random.Random(20260920)
cases=[]
def add(command,check): cases.append((command,check))
def close(a,b,tol=2e-9):
 assert abs(a-b)<=tol*max(1,abs(b)),(a,b)
def coords(p): return ' '.join(str(x) for q in p for x in q)
def hull(p):
 h=MultiPoint(p).convex_hull
 if h.geom_type=='Point':return [h.coords[0]]
 if h.geom_type=='LineString':return list(h.coords)
 a=list(h.exterior.coords)[:-1]
 if sum(a[i][0]*a[(i+1)%len(a)][1]-a[i][1]*a[(i+1)%len(a)][0] for i in range(len(a)))<0:a.reverse()
 return [tuple(map(int,q)) for q in a]
def geometry(p):
 return Point(p[0]) if len(p)==1 else LineString(p) if len(p)==2 else Polygon(p)
add('fixed',lambda s: s=='ok' or (_ for _ in ()).throw(AssertionError(s)))
pointsets=[[],[(1,2)],[(0,0),(0,0)],[(0,0),(4,0)],[(0,0),(1,0),(2,0)],[(0,0),(0,2),(2,2),(2,0)],[(0,0),(10**10,0),(0,10**10)],[(0,0),(0,0),(0,2),(2,2),(2,0)]]
for seed in range(50):
 for kind in ['circle','ellipse','spiked','heart','lemon']:
  a=[]
  for i in range(40):
   t=2*math.pi*(i+rng.uniform(-.1,.1))/40
   r=1+.1*math.sin(7*t) if kind=='spiked' else 1-.3*abs(math.sin(t)) if kind=='heart' else 1
   aspect=100 if kind=='ellipse' else 100000 if kind=='lemon' else 1
   a.append((round(10**6*r*math.cos(t)),round(10**6*r*math.sin(t)/aspect)))
  pointsets.append(a)
for _ in range(100):pointsets.append([(rng.randint(-100,100),rng.randint(-100,100)) for _ in range(rng.randint(2,30))])
for pts in pointsets:
 def check(s,p=pts):
  a=list(map(float,s.split()));n=len(p)
  d2=max((sum((x-y)**2 for x,y in zip(v,w)) for v in p for w in p),default=0)
  close(a[0],math.sqrt(d2));close(a[1],math.sqrt(d2))
  if n:
   i,j=map(int,a[2:4]);assert sum((p[i][k]-p[j][k])**2 for k in range(2))==d2
   sh=MultiPoint(p);close(a[4],sh.minimum_rotated_rectangle.area if sh.convex_hull.area else 0)
   close(a[7],minimum_bounding_radius(sh))
   for x,y in p:assert math.hypot(x-a[5],y-a[6])<=a[7]+1e-8*max(1,a[7])
   if a[4]>0:
    rect=Polygon(list(zip(a[8:16:2],a[9:16:2])))
    close(rect.area,a[4]);assert all(rect.distance(Point(q))<2e-8*max(1,math.sqrt(d2)) for q in p)
  else:assert a[:8]==[0,0,-1,-1,0,0,0,0]
  if n>=2:
   mind=min(sum((x-y)**2 for x,y in zip(v,w)) for v,w in itertools.combinations(p,2));close(a[16],mind,1e-15)
 add('poly '+str(len(pts))+' '+coords(pts),check)
# independent segments includes point-segment degeneration
for _ in range(400):
 p=[(rng.randint(-8,8),rng.randint(-8,8)) for _ in range(4)]
 if rng.random()<.2:p[1]=p[0]
 if rng.random()<.2:p[3]=p[2]
 a=Point(p[0]) if p[0]==p[1] else LineString(p[:2]);b=Point(p[2]) if p[2]==p[3] else LineString(p[2:])
 def ck(s,a=a,b=b):
  hit,d=map(float,s.split());assert hit==a.intersects(b),(a,b,hit);close(d,a.distance(b))
 add('seg '+coords(p),ck)
for _ in range(200):
 pts=hull([(rng.randint(-20,20),rng.randint(-20,20)) for _ in range(10)])
 q=(rng.randint(-25,25),rng.randint(-25,25));g=geometry(pts);pq=Point(q)
 exp=1 if g.boundary.covers(pq) else 2 if g.contains(pq) else 0
 add('in '+str(len(pts))+' '+coords(pts+[q]),lambda s,exp=exp: int(s)==exp or (_ for _ in ()).throw(AssertionError((s,exp))))
# concave polygon + horizontal edges + vertices
pts=[(0,0),(8,0),(8,8),(4,3),(0,8)]
for x in range(-1,10):
 for y in range(-1,10):
  g=Polygon(pts);q=Point(x,y);exp=1 if g.boundary.covers(q) else 2 if g.contains(q) else 0
  add('in 5 '+coords(pts+[(x,y)]),lambda s,exp=exp: int(s)==exp or (_ for _ in ()).throw(AssertionError((s,exp))))
for _ in range(120):
 a=hull([(rng.randint(-20,20),rng.randint(-20,20)) for _ in range(15)])
 b=hull([(rng.randint(-20,20),rng.randint(-20,20)) for _ in range(15)])
 sums=[(x+u,y+v) for x,y in a for u,v in b];expected=MultiPoint(sums).convex_hull.area
 add(f'sum {len(a)} {len(b)} '+coords(a+b),lambda s,e=expected:close(float(s),e))
 # shifted disjoint polygons (documented caliper precondition)
 b=[(x+80,y) for x,y in b];expected=geometry(a).distance(geometry(b))
 add(f'apart {len(a)} {len(b)} '+coords(a+b),lambda s,e=expected:close(float(s),e))
 # intersect their halfplanes with a box
 polys=[a,[(x-80,y) for x,y in b]]
 lines=[(p[i],p[(i+1)%len(p)]) for p in polys for i in range(len(p))]
 expected=Polygon(polys[0]).intersection(Polygon(polys[1])).area
 rng.shuffle(lines)
 add('hpi '+str(len(lines))+' '+coords([p for l in lines for p in l]),lambda s,e=expected:close(float(s),e))
# duplicate, parallel, contradictory halfplanes
for x in [0,1,3,5,10]:
 lines=[((0,0),(4,0)),((4,0),(4,4)),((4,4),(0,4)),((0,4),(0,0)),((x,4),(x,0))]
 add('hpi 5 '+coords([p for l in lines for p in l]),lambda s,e=max(0,4-x)*4:close(float(s),e))
# circle intersections and areas use high-resolution disk approximation, with explicit discretization bound.
res=2048
for _ in range(150):
 a=(rng.randint(-8,8),rng.randint(-8,8),rng.randint(1,8));b=(rng.randint(-8,8),rng.randint(-8,8),rng.randint(1,8))
 d2=(a[0]-b[0])**2+(a[1]-b[1])**2;sum2=(a[2]+b[2])**2;dif2=(a[2]-b[2])**2
 relation=-1 if d2==0 and a[2]==b[2] else 4 if d2>sum2 else 3 if d2==sum2 else 2 if d2>dif2 else 1 if d2==dif2 else 0
 intersections=-1 if relation==-1 else 0 if relation in [0,4] else 1 if relation in [1,3] else 2
 expected=Point(a[:2]).buffer(a[2],quad_segs=res).intersection(Point(b[:2]).buffer(b[2],quad_segs=res)).area
 bound=math.pi*(a[2]**2+b[2]**2)*(math.pi/(2*res))**2/6+1e-8
 def ck(s,a=a,b=b,r=relation,k=intersections,e=expected,bound=bound):
  v=list(map(float,s.split()));assert v[0]==r and v[1]==k and v[3]==max(0,k),(s,a,b,r,k)
  assert abs(v[2]-e)<=bound,(v[2],e,bound)
  count=int(v[3]);ps=list(zip(v[4:4+2*count:2],v[5:4+2*count:2]))
  for x,y in ps:
   close(math.hypot(x-a[0],y-a[1]),a[2]);close(math.hypot(x-b[0],y-b[1]),b[2])
  assert int(v[4+2*count])==max(0,r),(s,a,b)
 add('cir '+' '.join(map(str,a+b)),ck)
for _ in range(120):
 c=(rng.randint(-5,5),rng.randint(-5,5),rng.randint(1,8))
 pts=hull([(rng.randint(-15,15),rng.randint(-15,15)) for _ in range(10)])
 expected=Polygon(pts).intersection(Point(c[:2]).buffer(c[2],quad_segs=res)).area
 bound=math.pi*c[2]**2*(math.pi/(2*res))**2/6+1e-8
 add('disk '+' '.join(map(str,c))+f' {len(pts)} '+coords(pts),lambda s,e=expected,bound=bound:abs(float(s)-e)<=bound or (_ for _ in ()).throw(AssertionError((s,e,bound))))
for c,a,b in [((0,0,1),(-2,0),(2,0)),((0,0,1),(-2,1),(2,1)),((0,0,1),(-2,2),(2,2)),((0,0,1),(0,0),(0,0)),((0,0,0),(-2,0),(2,0))]:
 k=0 if a==b else 1-( ((a[0]-c[0])*(b[1]-a[1])-(a[1]-c[1])*(b[0]-a[0]))**2>c[2]**2*((a[0]-b[0])**2+(a[1]-b[1])**2) )+ ( ((a[0]-c[0])*(b[1]-a[1])-(a[1]-c[1])*(b[0]-a[0]))**2<c[2]**2*((a[0]-b[0])**2+(a[1]-b[1])**2) )
 add('cl '+' '.join(map(str,c))+' '+coords([a,b]),lambda s,k=k: int(s.split()[0])==k and int(s.split()[1])==k or (_ for _ in ()).throw(AssertionError((s,k))))
# Run batches; deterministic inputs retained in failure report.
for i in range(0,len(cases),100):
 batch=cases[i:i+100]
 run=subprocess.run([str(BIN)],input='\n'.join(c for c,_ in batch)+'\n',text=True,capture_output=True,timeout=30)
 assert run.returncode==0,run.stderr
 out=run.stdout.splitlines();assert len(out)==len(batch),(len(out),len(batch),run.stderr)
 for j,((c,check),s) in enumerate(zip(batch,out)):
  try:check(s)
  except Exception:
   print('FAIL',i+j,c,'OUTPUT',s);raise
print(f'PASS {len(cases)} deterministic cases; seed 20260920; Shapely independent oracles')
