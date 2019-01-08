
#include "vecmath.h"

#include "math/vecmat.h"
#include "render/3d.h"
#include "render/3dinternal.h"

namespace scripting {
namespace api {

vec3d_h::vec3d_h()
{

}

vec3d_h::vec3d_h(const vec3d_h& v)
{
	vec.xyz = v.vec.xyz;
}

vec3d_h::vec3d_h(vec3d_h&& v)
{
	vec.xyz = v.vec.xyz;
}

vec3d_h::vec3d_h(vec3d* v)
{
	vec.xyz = v->xyz;
}

vec3d_h& vec3d_h::operator=(const vec3d_h& v)
{
	vec.xyz = v.vec.xyz;
	return *this;
}

//**********OBJECT: orientation matrix
//WMC - So matrix can use vector, I define it up here.
ADE_OBJ(l_Vector, vec3d_h, "vector", "Vector object");

void matrix_h::ValidateAngles() {
	if (status == MatrixState::AnglesOutOfDate) {
		vm_extract_angles_matrix(&ang, &mtx);
		status = MatrixState::Fine;
	}
}
void matrix_h::ValidateMatrix() {
	if (status == MatrixState::MatrixOutOfdate) {
		vm_angles_2_matrix(&mtx, &ang);
		status = MatrixState::Fine;
	}
}
matrix_h::matrix_h() {
	mtx = vmd_identity_matrix;
	status = MatrixState::AnglesOutOfDate;
}
matrix_h::matrix_h(matrix* in) {
	mtx = *in;
	status = MatrixState::AnglesOutOfDate;
}
matrix_h::matrix_h(angles* in) {
	ang = *in;
	status = MatrixState::MatrixOutOfdate;
}
angles* matrix_h::GetAngles() {
	this->ValidateAngles();
	return &ang;
}
matrix* matrix_h::GetMatrix() {
	this->ValidateMatrix();
	return &mtx;
}
void matrix_h::SetStatus(MatrixState n_status) {
	status = n_status;
}

//LOOK LOOK LOOK LOOK LOOK LOOK
//IMPORTANT!!!:
//LOOK LOOK LOOK LOOK LOOK LOOK
//Don't forget to set status appropriately when you change ang or mtx.

ADE_OBJ(l_Matrix, matrix_h, "orientation", "Orientation matrix object");

ADE_INDEXER(l_Matrix,
			"p,b,h or 1-9",
			"Orientation component - pitch, bank, heading, or index into 3x3 matrix (1-9)",
			"number",
			"Number at the specified index, or 0 if index is invalid.") {
	matrix_h* mh;
	const char* s = nullptr;
	float newval = 0.0f;
	int numargs = ade_get_args(L, "os|f", l_Matrix.GetPtr(&mh), &s, &newval);

	if (!numargs || s[1] != '\0') {
		return ade_set_error(L, "f", 0.0f);
	}

	int idx = 0;
	if (s[0] == 'p') {
		idx = -1;
	} else if (s[0] == 'b') {
		idx = -2;
	} else if (s[0] == 'h') {
		idx = -3;
	} else if (atoi(s)) {
		idx = atoi(s);
	}

	if (idx < -3 || idx == 0 || idx > 9) {
		return ade_set_error(L, "f", 0.0f);
	}

//Handle out of date stuff.
	float* val = NULL;
	if (idx < 0) {
		angles* ang = mh->GetAngles();

		if (idx == -1) {
			val = &ang->p;
		}
		if (idx == -2) {
			val = &ang->b;
		}
		if (idx == -3) {
			val = &ang->h;
		}
	} else {
		idx--;    //Lua->FS2
		val = &mh->GetMatrix()->a2d[idx / 3][idx % 3];
	}

	if (ADE_SETTING_VAR && *val != newval) {
//WMC - I figure this is quicker
//than just assuming matrix or angles is diff
//and recalculating every time.

		if (idx < 0) {
			mh->SetStatus(MatrixState::MatrixOutOfdate);
		} else {
			mh->SetStatus(MatrixState::MatrixOutOfdate);
		}

//Might as well put this here
		*val = newval;
	}

	return ade_set_args(L, "f", *val);
}

ADE_FUNC(__mul,
		 l_Matrix,
		 "orientation",
		 "Multiplies two matrix objects)",
		 "orientation",
		 "matrix, or empty matrix if unsuccessful") {
	matrix_h* mha = NULL, * mhb = NULL;
	if (!ade_get_args(L, "oo", l_Matrix.GetPtr(&mha), l_Matrix.GetPtr(&mhb))) {
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));
	}

	matrix mr;

	vm_matrix_x_matrix(&mr, mha->GetMatrix(), mhb->GetMatrix());

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&mr)));
}

ADE_FUNC(__tostring,
		 l_Matrix,
		 NULL,
		 "Converts a matrix to a string with format \"[r1c1 r2c1 r3c1 | r1c2 r2c2 r3c2| r1c3 r2c3 r3c3]\"",
		 "string",
		 "Formatted string or \"<NULL\"") {
	matrix_h* mh;
	if (!ade_get_args(L, "o", l_Matrix.GetPtr(&mh))) {
		return ade_set_error(L, "s", "<NULL>");
	}

	char buf[128];
	matrix *m = mh->GetMatrix();
	sprintf(buf, "[%f %f %f | %f %f %f | %f %f %f]", vm_matrix_get_a1d(m, 0), vm_matrix_get_a1d(m, 1), vm_matrix_get_a1d(m, 2), vm_matrix_get_a1d(m, 3), vm_matrix_get_a1d(m, 4), vm_matrix_get_a1d(m, 5), vm_matrix_get_a1d(m, 6), vm_matrix_get_a1d(m, 7), vm_matrix_get_a1d(m, 8));

	return ade_set_args(L, "s", buf);
}

ADE_FUNC(getInterpolated,
		 l_Matrix,
		 "orientation Final, number Factor",
		 "Returns orientation that has been interpolated to Final by Factor (0.0-1.0)",
		 "orientation",
		 "Interpolated orientation, or null orientation on failure") {
	matrix_h* oriA = NULL;
	matrix_h* oriB = NULL;
	float factor = 0.0f;
	if (!ade_get_args(L, "oof", l_Matrix.GetPtr(&oriA), l_Matrix.GetPtr(&oriB), &factor)) {
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));
	}

	matrix* A = oriA->GetMatrix();
	matrix* B = oriB->GetMatrix();
	matrix final = vmd_identity_matrix;

//matrix subtraction & scaling
	for (int i = 0; i < 9; i++) {
		vm_matrix_set_a1d(&final, i, vm_matrix_get_a1d(A, i) + (vm_matrix_get_a1d(B, i) - vm_matrix_get_a1d(A, i) * factor));
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&final)));
}

ADE_FUNC(getTranspose,
		 l_Matrix,
		 NULL,
		 "Returns a transpose version of the specified orientation",
		 "orientation",
		 "Transpose matrix, or null orientation on failure") {
	matrix_h* mh = NULL;
	if (!ade_get_args(L, "o", l_Matrix.GetPtr(&mh))) {
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));
	}

	matrix final = *mh->GetMatrix();
	vm_transpose(&final);

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&final)));
}


ADE_FUNC(rotateVector,
		 l_Matrix,
		 "vector Input",
		 "Returns rotated version of given vector",
		 "vector",
		 "Rotated vector, or empty vector on error") {
	matrix_h* mh;
	vec3d_h* v3;
	if (!ade_get_args(L, "oo", l_Matrix.GetPtr(&mh), l_Vector.GetPtr(&v3))) {
		return ade_set_error(L, "o", l_Vector.Set(vec3d_h(&vmd_zero_vector)));
	}

	vec3d_h v3r;
	vm_vec_rotate(&v3r.vec, &v3->vec, mh->GetMatrix());

	return ade_set_args(L, "o", l_Vector.Set(v3r));
}

ADE_FUNC(unrotateVector,
		 l_Matrix,
		 "vector Input",
		 "Returns unrotated version of given vector",
		 "vector",
		 "Unrotated vector, or empty vector on error") {
	matrix_h* mh;
	vec3d_h* v3;
	if (!ade_get_args(L, "oo", l_Matrix.GetPtr(&mh), l_Vector.GetPtr(&v3))) {
		return ade_set_error(L, "o", l_Vector.Set(vec3d_h(&vmd_zero_vector)));
	}

	vec3d_h v3r;
	vm_vec_unrotate(&v3r.vec, &v3->vec, mh->GetMatrix());

	return ade_set_args(L, "o", l_Vector.Set(v3r));
}

ADE_FUNC(getUvec,
		 l_Matrix,
		 NULL,
		 "Returns the vector that points up (0,1,0 unrotated by this matrix)",
		 "vector",
		 "Vector or null vector on error") {
	matrix_h* mh = NULL;
	if (!ade_get_args(L, "o", l_Matrix.GetPtr(&mh))) {
		return ade_set_error(L, "o", l_Vector.Set(vec3d_h(&vmd_zero_vector)));
	}

	return ade_set_args(L, "o", l_Vector.Set(vec3d_h(&mh->GetMatrix()->vec.uvec)));
}

ADE_FUNC(getFvec,
		 l_Matrix,
		 NULL,
		 "Returns the vector that points to the front (0,0,1 unrotated by this matrix)",
		 "vector",
		 "Vector or null vector on error") {
	matrix_h* mh = NULL;
	if (!ade_get_args(L, "o", l_Matrix.GetPtr(&mh))) {
		return ade_set_error(L, "o", l_Vector.Set(vec3d_h(&vmd_zero_vector)));
	}

	return ade_set_args(L, "o", l_Vector.Set(vec3d_h(&mh->GetMatrix()->vec.fvec)));
}

ADE_FUNC(getRvec,
		 l_Matrix,
		 NULL,
		 "Returns the vector that points to the right (1,0,0 unrotated by this matrix)",
		 "vector",
		 "Vector or null vector on error") {
	matrix_h* mh = NULL;
	if (!ade_get_args(L, "o", l_Matrix.GetPtr(&mh))) {
		return ade_set_error(L, "o", l_Vector.Set(vec3d_h(&vmd_zero_vector)));
	}

	return ade_set_args(L, "o", l_Vector.Set(vec3d_h(&mh->GetMatrix()->vec.rvec)));
}


ADE_INDEXER(l_Vector,
			"x,y,z or 1-3",
			"Vector component",
			"number",
			"Value at index, or 0 if vector handle is invalid") {
	vec3d_h* v3;
	const char* s = nullptr;
	float newval = 0.0f;
	int numargs = ade_get_args(L, "os|f", l_Vector.GetPtr(&v3), &s, &newval);

	if (!numargs || s[1] != '\0') {
		return ade_set_error(L, "f", 0.0f);
	}

	int idx = -1;
	if (s[0] == 'x' || s[0] == '1') {
		idx = 0;
	} else if (s[0] == 'y' || s[0] == '2') {
		idx = 1;
	} else if (s[0] == 'z' || s[0] == '3') {
		idx = 2;
	}

	if (idx < 0 || idx > 3) {
		return ade_set_error(L, "f", 0.0f);
	}

	if (ADE_SETTING_VAR) {
		v3->vec.a1d[idx] = newval;
	}

	return ade_set_args(L, "f", v3->vec.a1d[idx]);
}

ADE_FUNC(__add,
		 l_Vector,
		 "number/vector",
		 "Adds vector by another vector, or adds all axes by value",
		 "vector",
		 "Final vector, or null vector if error occurs") {
	vec3d_h v3(&vmd_zero_vector);
	if (lua_isnumber(L, 1) || lua_isnumber(L, 2)) {
		float f;
		if ((lua_isnumber(L, 1) && ade_get_args(L, "fo", &f, l_Vector.Get(&v3)))
			|| (lua_isnumber(L, 2) && ade_get_args(L, "of", l_Vector.Get(&v3), &f))) {
			v3.vec.xyz.x += f;
			v3.vec.xyz.y += f;
			v3.vec.xyz.z += f;
		}
	} else {
		vec3d_h v3b;
		//WMC - doesn't really matter which is which
		if (ade_get_args(L, "oo", l_Vector.Get(&v3), l_Vector.Get(&v3b))) {
			vm_vec_add2(&v3.vec, &v3b.vec);
		}
	}
	return ade_set_args(L, "o", l_Vector.Set(v3));
}

ADE_FUNC(__sub,
		 l_Vector,
		 "number/vector",
		 "Subtracts vector from another vector, or subtracts all axes by value",
		 "vector",
		 "Final vector, or null vector if error occurs") {
	vec3d_h v3(&vmd_zero_vector);
	if (lua_isnumber(L, 1) || lua_isnumber(L, 2)) {
		float f;
		if ((lua_isnumber(L, 1) && ade_get_args(L, "fo", &f, l_Vector.Get(&v3)))
			|| (lua_isnumber(L, 2) && ade_get_args(L, "of", l_Vector.Get(&v3), &f))) {
			v3.vec.xyz.x += f;
			v3.vec.xyz.y += f;
			v3.vec.xyz.z += f;
		}
	} else {
		vec3d_h v3b;
		//WMC - doesn't really matter which is which
		if (ade_get_args(L, "oo", l_Vector.Get(&v3), l_Vector.Get(&v3b))) {
			vm_vec_sub2(&v3.vec, &v3b.vec);
		}
	}

	return ade_set_args(L, "o", l_Vector.Set(v3));
}

ADE_FUNC(__mul,
		 l_Vector,
		 "number/vector",
		 "Scales vector object (Multiplies all axes by number), or multiplies each axes by the other vector's axes.",
		 "vector",
		 "Final vector, or null vector if error occurs") {
	vec3d_h v3(&vmd_zero_vector);
	if (lua_isnumber(L, 1) || lua_isnumber(L, 2)) {
		float f;
		if ((lua_isnumber(L, 1) && ade_get_args(L, "fo", &f, l_Vector.Get(&v3)))
			|| (lua_isnumber(L, 2) && ade_get_args(L, "of", l_Vector.Get(&v3), &f))) {
			vm_vec_scale(&v3.vec, f);
		}
	} else {
		vec3d_h* v1 = NULL;
		vec3d_h* v2 = NULL;
		if (!ade_get_args(L, "oo", l_Vector.GetPtr(&v1), l_Vector.GetPtr(&v2))) {
			return ade_set_args(L, "o", l_Vector.Set(vec3d_h(&vmd_zero_vector)));
		}

		v3.vec.xyz.x = v1->vec.xyz.x * v2->vec.xyz.x;
		v3.vec.xyz.y = v1->vec.xyz.y * v2->vec.xyz.y;
		v3.vec.xyz.z = v1->vec.xyz.z * v2->vec.xyz.z;
	}

	return ade_set_args(L, "o", l_Vector.Set(v3));
}

ADE_FUNC(__div,
		 l_Vector,
		 "number/vector",
		 "Scales vector object (Divide all axes by number), or divides each axes by the dividing vector's axes.",
		 "vector",
		 "Final vector, or null vector if error occurs") {
	vec3d_h v3(&vmd_zero_vector);
	if (lua_isnumber(L, 1) || lua_isnumber(L, 2)) {
		float f;
		if ((lua_isnumber(L, 1) && ade_get_args(L, "fo", &f, l_Vector.Get(&v3)))
			|| (lua_isnumber(L, 2) && ade_get_args(L, "of", l_Vector.Get(&v3), &f))) {
			vm_vec_scale(&v3.vec, 1.0f / f);
		}
	} else {
		vec3d_h* v1 = NULL;
		vec3d_h* v2 = NULL;
		if (!ade_get_args(L, "oo", l_Vector.GetPtr(&v1), l_Vector.GetPtr(&v2))) {
			return ade_set_args(L, "o", l_Vector.Set(vec3d_h(&vmd_zero_vector)));
		}

		v3.vec.xyz.x = v1->vec.xyz.x / v2->vec.xyz.x;
		v3.vec.xyz.y = v1->vec.xyz.y / v2->vec.xyz.y;
		v3.vec.xyz.z = v1->vec.xyz.z / v2->vec.xyz.z;
	}

	return ade_set_args(L, "o", l_Vector.Set(v3));
}


ADE_FUNC(__tostring,
		 l_Vector,
		 NULL,
		 "Converts a vector to string with format \"(x,y,z)\"",
		 "string",
		 "Vector as string, or empty string if handle is invalid") {
	vec3d_h* v3;
	if (!ade_get_args(L, "o", l_Vector.GetPtr(&v3))) {
		return ade_set_error(L, "s", "");
	}

	char buf[128];
	sprintf(buf, "(%f,%f,%f)", v3->vec.xyz.x, v3->vec.xyz.y, v3->vec.xyz.z);

	return ade_set_args(L, "s", buf);
}

ADE_FUNC(getOrientation,
		 l_Vector,
		 NULL,
		 "Returns orientation object representing the direction of the vector. Does not require vector to be normalized.",
		 "orientation",
		 "Orientation object, or null orientation object if handle is invalid") {
	vec3d_h v3;
	if (!ade_get_args(L, "o", l_Vector.Get(&v3))) {
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));
	}

	matrix mt = vmd_identity_matrix;

	vm_vec_normalize_safe(&v3.vec);
	vm_vector_2_matrix_norm(&mt, &v3.vec);
	matrix_h mh(&mt);

	return ade_set_args(L, "o", l_Matrix.Set(mh));
}

ADE_FUNC(getMagnitude,
		 l_Vector,
		 NULL,
		 "Returns the magnitude of a vector (Total regardless of direction)",
		 "number",
		 "Magnitude of vector, or 0 if handle is invalid") {
	vec3d_h* v3;
	if (!ade_get_args(L, "o", l_Vector.GetPtr(&v3))) {
		return ade_set_error(L, "f", 0.0f);
	}

	return ade_set_args(L, "f", vm_vec_mag(&v3->vec));
}

ADE_FUNC(getDistance, l_Vector, "Vector", "Distance", "number", "Returns distance from another vector") {
	vec3d_h* v3a, * v3b;
	if (!ade_get_args(L, "oo", l_Vector.GetPtr(&v3a), l_Vector.GetPtr(&v3b))) {
		return ade_set_error(L, "f", 0.0f);
	}

	return ade_set_args(L, "f", vm_vec_dist(&v3a->vec, &v3b->vec));
}

ADE_FUNC(getDotProduct,
		 l_Vector,
		 "vector OtherVector",
		 "Returns dot product of vector object with vector argument",
		 "number",
		 "Dot product, or 0 if a handle is invalid") {
	vec3d_h* v3a, * v3b;
	if (!ade_get_args(L, "oo", l_Vector.GetPtr(&v3a), l_Vector.GetPtr(&v3b))) {
		return ade_set_error(L, "f", 0.0f);
	}

	return ade_set_args(L, "f", vm_vec_dot(&v3a->vec, &v3b->vec));
}

ADE_FUNC(getCrossProduct,
		 l_Vector,
		 "vector OtherVector",
		 "Returns cross product of vector object with vector argument",
		 "vector",
		 "Cross product, or null vector if a handle is invalid") {
	vec3d_h* v3a, * v3b;
	if (!ade_get_args(L, "oo", l_Vector.GetPtr(&v3a), l_Vector.GetPtr(&v3b))) {
		return ade_set_error(L, "o", l_Vector.Set(vec3d_h(&vmd_zero_vector)));
	}

	vec3d_h v3r;
	vm_vec_cross(&v3r.vec, &v3a->vec, &v3b->vec);

	return ade_set_args(L, "o", l_Vector.Set(v3r));
}

ADE_FUNC(getScreenCoords,
		 l_Vector,
		 NULL,
		 "Gets screen cordinates of a world vector",
		 "number,number",
		 "X (number), Y (number), or false if off-screen") {
	vec3d_h v3;
	if (!ade_get_args(L, "o", l_Vector.Get(&v3))) {
		return ADE_RETURN_NIL;
	}

	vertex vtx;
	bool do_g3 = G3_count < 1;
	if (do_g3)
		g3_start_frame(1);

	g3_rotate_vertex(&vtx, &v3.vec);
	g3_project_vertex(&vtx);

	if (do_g3)
		g3_end_frame();

	if (vtx.flags & PF_OVERFLOW) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "ff", vtx.screen.xyw.x, vtx.screen.xyw.y);
}

ADE_FUNC(getNormalized,
		 l_Vector,
		 NULL,
		 "Returns a normalized version of the vector",
		 "vector",
		 "Normalized Vector, or NIL if invalid") {
	vec3d_h v3;
	if (!ade_get_args(L, "o", l_Vector.Get(&v3))) {
		return ADE_RETURN_NIL;
	}

	vm_vec_normalize(&v3.vec);

	return ade_set_args(L, "o", l_Vector.Set(v3));
}

}
}
