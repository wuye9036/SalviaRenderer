#include <eflib/include/math/collision_detection.h>
#include <eflib/include/math/math.h>

namespace eflib{
	template<class T>
	void min_max(const T& _1, const T& _2, const T& _3, T& min, T& max)
	{
		min = max = _1;
		if(_2 < min) min = _2;
		if(_2 > max) max = _2;
		if(_3 < min) min = _3;
		if(_3 > max) max = _3;
	}

	bool plane_box_overlap(const vec4& normal, const vec4& vert, const vec4& maxbox)	// -NJMP-
	{
		int q;
		float v;
		vec4 vmin,vmax;
		for(q = 0; q < 3; q++){
			v=vert[q];					// -NJMP-
			if(normal[q]>0.0f){
				vmin[q]= - maxbox[q] - v;	// -NJMP-
				vmax[q]=  maxbox[q] - v;	// -NJMP-
			}else{
				vmin[q]= maxbox[q] - v;	// -NJMP-
				vmax[q]=-maxbox[q] - v;	// -NJMP-
			}
		}

		if(dot_prod3(normal.xyz(), vmin.xyz()) > 0.0f) return false;	// -NJMP-
		if(dot_prod3(normal.xyz(), vmax.xyz()) >= 0.0f) return true;	// -NJMP-
		return false;
	}

	/*======================== X-tests ========================*/

#define AXISTEST_X01(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			       	   \
	p2 = a*v2[Y] - b*v2[Z];			       	   \
	if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return false;

#define AXISTEST_X2(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			           \
	p1 = a*v1[Y] - b*v1[Z];			       	   \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return false;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p2 = -a*v2[X] + b*v2[Z];	       	       	   \
	if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return false;



#define AXISTEST_Y1(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p1 = -a*v1[X] + b*v1[Z];	     	       	   \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;

/*======================== Z-tests ========================*/
#define AXISTEST_Z12(a, b, fa, fb)			   \
	p1 = a*v1[X] - b*v1[Y];			           \
	p2 = a*v2[X] - b*v2[Y];			       	   \
	if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)			   \
	p0 = a*v0[X] - b*v0[Y];				   \
	p1 = a*v1[X] - b*v1[Y];			           \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
	if(min>rad || max<-rad) return 0;

	bool is_tri_cube_overlap(const AABB_3D& box, const vec4& tv0, const vec4& tv1, const vec4& tv2)
	{
		vec4 boxcenter, boxhalfsize;
		box.get_center_size(boxcenter, boxhalfsize);
		const int X = 0;
		const int Y = 1;
		const int Z = 2;

		/*    use separating axis theorem to test overlap between triangle and box */
		/*    need to test for overlap in these directions: */
		/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
		/*       we do not even need to test these) */
		/*    2) normal of the triangle */
		/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
		/*       this gives 3x3=9 more tests */
		//   float axis[3];
		vec4 v0(tv0 - boxcenter);
		vec4 v1(tv1 - boxcenter);
		vec4 v2(tv2 - boxcenter);
		
		float min,max,p0,p1,p2,rad,fex,fey,fez;		// -NJMP- "d" local variable removed

		/* compute triangle edges */
		vec4 e0(v1 - v0);      /* tri edge 0 */
		vec4 e1(v2 - v1);      /* tri edge 1 */
		vec4 e2(v0 - v2);      /* tri edge 2 */

		/* Bullet 3:  */
		/*  test the 9 tests first (this was faster) */
		fex = fabsf(e0[X]);
		fey = fabsf(e0[Y]);
		fez = fabsf(e0[Z]);

		AXISTEST_X01(e0[Z], e0[Y], fez, fey);
		AXISTEST_Y02(e0[Z], e0[X], fez, fex);
		AXISTEST_Z12(e0[Y], e0[X], fey, fex);

		fex = fabsf(e1[X]);
		fey = fabsf(e1[Y]);
		fez = fabsf(e1[Z]);

		AXISTEST_X01(e1[Z], e1[Y], fez, fey);
		AXISTEST_Y02(e1[Z], e1[X], fez, fex);
		AXISTEST_Z0(e1[Y], e1[X], fey, fex);

		fex = fabsf(e2[X]);
		fey = fabsf(e2[Y]);
		fez = fabsf(e2[Z]);

		AXISTEST_X2(e2[Z], e2[Y], fez, fey);
		AXISTEST_Y1(e2[Z], e2[X], fez, fex);
		AXISTEST_Z12(e2[Y], e2[X], fey, fex);

		/* Bullet 1: */
		/*  first test overlap in the {x,y,z}-directions */
		/*  find min, max of the triangle each direction, and test for overlap in */
		/*  that direction -- this is equivalent to testing a minimal AABB around */
		/*  the triangle against the AABB */

		/* test in X-direction */
		min_max(v0[X],v1[X],v2[X],min,max);
		if(min>boxhalfsize[X] || max<-boxhalfsize[X]) return false;

		/* test in Y-direction */
		min_max(v0[Y],v1[Y],v2[Y],min,max);
		if(min>boxhalfsize[Y] || max<-boxhalfsize[Y]) return false;

		/* test in Z-direction */
		min_max(v0[Z],v1[Z],v2[Z],min,max);
		if(min>boxhalfsize[Z] || max<-boxhalfsize[Z]) return false;

		/* Bullet 2: */
		/*  test if the box intersects the plane of the triangle */
		/*  compute plane equation of triangle: normal*x+d=0 */
		vec4 normal;
		normal.xyz() = cross_prod3(e0.xyz(), e1.xyz());
		if(!plane_box_overlap(normal, v0, boxhalfsize)) return false;	// -NJMP-

		return true;   /* box and triangle overlaps */
	}
}