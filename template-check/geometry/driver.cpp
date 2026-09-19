#include "library.hpp"
int main() {
 rng.seed(20260920);
 cout << setprecision(20);
 string cmd;
 while(cin>>cmd) {
  if(cmd=="hull" || cmd=="hull_db") {
   int strict, mode,n;cin>>strict>>mode>>n;
   if(cmd=="hull") {
    vector<Point<ll>> p(n);for(auto &a:p)cin>>a;
    auto h=make_convex_hull(p,strict,(HullMode)mode);
    cout<<h.size();for(auto a:h)cout<<" "<<a.x<<" "<<a.y;cout<<"\n";
   } else {
    vector<Point<DB>> p(n);for(auto &a:p)cin>>a;
    auto h=make_convex_hull(p,strict,(HullMode)mode);
    cout<<h.size();for(auto a:h)cout<<" "<<a.x<<" "<<a.y;cout<<"\n";
   }
  } else if(cmd=="poly") {
   int n;cin>>n;vector<Point<ll>> p(n);for(int i=0;i<n;++i){cin>>p[i];p[i].id=i;}
   auto h=make_convex_hull(p,true,HULL_FULL),w=make_convex_hull(p,false,HULL_FULL);
   auto far=furthest_pair_of_point(p);
   vector<Point<DB>> f;for(auto a:p)f.push_back(a);
   auto rect=minimum_rectangle_cover(f);auto c=min_circle_cover(p);
   cout<<convex_diameter(h)<<" "<<convex_diameter(w)<<" "<<far.first<<" "<<far.second<<" "<<rect.first<<" "<<c.c.x<<" "<<c.c.y<<" "<<c.r;
   for(auto a:rect.second)cout<<" "<<a.x<<" "<<a.y;
   if(n>=2){copy(p.begin(),p.end(),A);sort(A,A+n);auto v=dfs(0,n-1);cout<<" "<<(long double)v.d;}
   cout<<"\n";
  } else if(cmd=="in") {
   int n;cin>>n;vector<Point<ll>> p(n);for(auto &a:p)cin>>a;Point<ll> q;cin>>q;cout<<isInPolygon(q,p)<<"\n";
  } else if(cmd=="seg") {
   Line<ll>a,b;cin>>a>>b;cout<<isSegIntersect(a,b)<<" "<<distanceSS(a,b)<<"\n";
  } else if(cmd=="hpi") {
   int n;cin>>n;vector<Line<DB>> a(n);for(auto &l:a)cin>>l;
   auto p=half_plane_cut(a);cout<<polygonArea(p)<<"\n";
  } else if(cmd=="sum" || cmd=="apart") {
   int n,m;cin>>n>>m;vector<Point<ll>>a(n),b(m);for(auto &p:a)cin>>p;for(auto &p:b)cin>>p;
   if(cmd=="sum") cout<<polygonArea(minkowski(a,b))<<"\n";
   else cout<<min(cal_min_dis_two_convex(a,b),cal_min_dis_two_convex(b,a))<<"\n";
  } else if(cmd=="disk") {
   Circle<DB>c;int n;cin>>c>>n;vector<Point<ll>>p(n);for(auto &a:p)cin>>a;cout<<polygon_circle_intersect_area(c,p)<<"\n";
  } else if(cmd=="cir") {
   Circle<ll>a,b;cin>>a>>b;
   auto pts=getCrossPointsCC(a,b);auto tang=getCommonTangentPoints(a,b);
   cout<<circle_relation(a,b)<<" "<<circle_circle_relation(a,b)<<" "<<two_circle_intersect_area(a,b)<<" "<<pts.size();
   for(auto p:pts)cout<<" "<<p.x<<" "<<p.y;
   cout<<" "<<tang.size();for(auto p:tang)cout<<" "<<p.x<<" "<<p.y;cout<<"\n";
  } else if(cmd=="cl") {
   Circle<ll>c;Line<ll>l;cin>>c>>l;auto pts=getCrossPointsCL(c,l);cout<<circle_line_relation(c,l)<<" "<<pts.size();for(auto p:pts)cout<<" "<<p.x<<" "<<p.y;cout<<"\n";
  } else if(cmd=="fixed") {
   using P=Point<ll>; using F=Point<DB>;
   static_assert(is_same_v<decltype(cross(P(),P())),i128>);
   static_assert(is_same_v<decltype(cross(P(),F())),DB>);
   static_assert(is_same_v<decltype(dot(Point<i128>(),Point<i128>())),i128>);
   static_assert(is_convertible_v<P,F> && !is_convertible_v<F,P>);
   static_assert(is_same_v<decltype(distancePPLinf(P(),F())),DB>);
   assert(sqrtL(i128(1)<<100)==(DB)(1LL<<50) && sqrtL(i128(16))==4 && sqrtL(i128(0))==0);
   assert(abs(sqrtL(i128(15))-sqrtl(15))<eps && sqrtL((DB)2.25)==1.5);
   assert((P(1,2)+F(.5,0))==F(1.5,2) && (F(1,0)-P(1,0))==F() && (P(1,2)*.5)==F(.5,1) && (P(3,3)/2)==P(1,1));
   P p(3,4);p.id=12;F f=p;assert(f.id==12 && (p+p).id==-1);
   assert(p.abs()==5 && p.normL1()==7 && p.normLinf()==4);
   assert(distancePP(p,F(3,4))==0 && dot(P(),p,F(p))==25);
   assert(cross(p,F(0,1))==3 && cross(P(),p,F(0,1))==3);
   Line<ll>l(P(0,0),P(2,0));Line<DB>lf=l;
   assert(project(F(1,2),l)==F(1,0));assert(reflect(p,lf)==F(3,-4));
   assert(isSegIntersect(Line<ll>(P(1,0),P(1,0)),l));
   assert(distancePPLinf(P(),F(.5,.25))==.5);
   F sa(0,2),sb(eps*.75,1),sc(eps*1.5,0);
   assert(sa<sb && sb<sc && sa<sc);
   assert(isParallel(l,lf) && isOrthogonal(l,Line<DB>(F(),F(0,1))));
   assert(getintersect(l,Line<DB>(F(1,-1),F(1,1)))==F(1,0));
   assert(CCW(P(0,0),l)==0 && CCW(P(-1,0),l)==2 && CCW(P(3,0),l)==-2);
   assert(CCW(P(0,1),l)==1 && CCW(P(0,-1),l)==-1);
   assert(abs(ccw_angle(F(1,0),P(0,1))-PI/2)<eps);
   Circle<DB>c(F(),2);assert(c.point_at(0)==F(2,0));assert(abs(c.theta_at(F(0,2))-PI/2)<eps);
   assert(abs(c.arc_length(PI)-2*PI)<eps && abs(c.chord_length(PI)-4)<eps);
   assert(abs(c.sagitta(PI)-2)<eps && abs(c.sector_area(PI)-2*PI)<eps && abs(c.segment_area(PI)-2*PI)<eps);
   auto inc=incircle_triangle(F(),F(4,0),F(0,3));assert(inc.c==F(1,1)&&abs(inc.r-1)<eps);
   auto out=outcircle_triangle(F(),F(4,0),F(0,3));assert(out.c==F(2,1.5)&&abs(out.r-2.5)<eps);
   assert(incircle_triangle(F(),F(1,0),F(2,0)).r==0);
   assert(outcircle_triangle(F(),F(1,0),F(2,0)).r==-1);
   assert(circle_relation(Circle<ll>(P(),1),Circle<ll>(P(),1))==COINCIDENT);
   assert(circle_circle_relation(Circle<ll>(P(),1),Circle<ll>(P(),1))==-1);
   assert(getCrossPointsCC(Circle<ll>(P(),1),Circle<ll>(P(),1)).empty());
   assert(getTangentPoints(P(),Circle<ll>(P(),1)).empty());
   assert(getTangentPoints(P(1,0),Circle<ll>(P(),1)).size()==1);
   assert(abs(distancePL(P(2,0),gen_line_from_general(1,-1,-2)))<eps);
   auto tp=getTangentPoints(P(2,0),Circle<ll>(P(),1));assert(tp.size()==2);
   vector<P>square{P(0,0),P(2,0),P(2,2),P(0,2)};
   assert(pick_boundary(square)==8&&pick_interior(square)==1&&polygon_perimeter(square)==8);
   assert(isConvex(square));reorder_polygon(square);assert(square[0]==P(0,0));
   assert(abs(regular_polygon_area(4,2)-4)<eps);
   vector<P>pol{P(0,-1),P(1,0),P(0,1),P(-1,0)};reverse(pol.begin(),pol.end());polar_angle_sort(pol);assert(pol[0]==P(0,-1));
   for(int n:{0,1,2,3,10})for(bool weak:{false,true}){
    auto q=gen_convex_polygon<ll>(n,weak,100000);assert((int)q.size()==n);
    auto r=gen_convex_polygon<DB>(n,weak,100000);assert((int)r.size()==n);
    if(n>2){assert(isConvex(q,!weak));assert(isConvex(r,!weak));}
   }
   cout<<"ok\n";
  }
 }
}
