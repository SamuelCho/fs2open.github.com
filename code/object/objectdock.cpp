/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



#include "math/bitarray.h"
#include "math/vecmat.h"
#include "mission/missionparse.h"
#include "object/object.h"
#include "object/objectdock.h"
#include "ship/ship.h"




// helper prototypes

void dock_evaluate_tree(object *objp, dock_function_info *infop, void (*function)(object *, dock_function_info *), ubyte *visited_bitstring);
void dock_move_docked_children_tree(object *objp, object *parent_objp);
void dock_count_total_docked_objects_helper(object *objp, dock_function_info *infop);
void dock_check_find_docked_object_helper(object *objp, dock_function_info *infop);
void dock_calc_docked_mins_maxs_helper(object *objp, dock_function_info *infop);
void dock_calc_docked_center_of_mass_helper(object *objp, dock_function_info *infop);
void dock_calc_total_docked_mass_helper(object *objp, dock_function_info *infop);
void dock_calc_max_cross_sectional_radius_squared_perpendicular_to_line_helper(object *objp, dock_function_info *infop);
void dock_calc_max_semilatus_rectum_squared_parallel_to_directrix_helper(object *objp, dock_function_info *infop);
void dock_find_max_speed_helper(object *objp, dock_function_info *infop);
void dock_find_max_fspeed_helper(object *objp, dock_function_info *infop);


// management prototypes

bool dock_check_assume_hub();
object *dock_get_hub(object *objp);

void dock_add_instance(object *objp, int dockpoint, object *other_objp);
void dock_remove_instance(object *objp, object *other_objp);
dock_instance *dock_find_instance(object *objp, object *other_objp);
dock_instance *dock_find_instance(object *objp, int dockpoint);
int dock_count_instances(object *objp);



object *dock_get_first_docked_object(object *objp)
{
	Assert(objp != NULL);

	// are we docked?
	if (!object_is_docked(objp))
		return NULL;

	return objp->dock_list->docked_objp;
}

bool dock_check_docked_one_on_one(object *objp)
{
	Assert(objp != NULL);

	// we must be docked
	if (!object_is_docked(objp))
		return false;
	
	// our dock list must contain only one object
	if (objp->dock_list->next != NULL)
		return false;

	// the other guy's dock list must contain only one object
	if (dock_get_first_docked_object(objp)->dock_list->next != NULL)
		return false;

	// debug check to make sure that we're docked to each other
	Assert(objp == dock_get_first_docked_object(objp)->dock_list->docked_objp);
	
	// success
	return true;
}

int dock_count_direct_docked_objects(object *objp)
{
	Assert(objp != NULL);
	return dock_count_instances(objp);
}

int dock_count_total_docked_objects(object *objp)
{
	Assert(objp != NULL);

	dock_function_info dfi;

	dock_evaluate_all_docked_objects(objp, &dfi, dock_count_total_docked_objects_helper);

	return dfi.maintained_variables.int_value;
}

bool dock_check_find_direct_docked_object(object *objp, object *other_objp)
{
	Assert(objp != NULL);
	Assert(other_objp != NULL);

	return (dock_find_instance(objp, other_objp) != NULL);
}

bool dock_check_find_docked_object(object *objp, object *other_objp)
{
	Assert(objp != NULL);
	Assert(other_objp != NULL);

	dock_function_info dfi;
	dfi.parameter_variables.objp_value = other_objp;

	dock_evaluate_all_docked_objects(objp, &dfi, dock_check_find_docked_object_helper);

	return dfi.maintained_variables.bool_value;
}

object *dock_find_object_at_dockpoint(object *objp, int dockpoint)
{
	Assert(objp != NULL);

	dock_instance *result = dock_find_instance(objp, dockpoint);
	
	if (result == NULL)
		return NULL;
	else
		return result->docked_objp;
}

int dock_find_dockpoint_used_by_object(object *objp, object *other_objp)
{
	Assert(objp != NULL);
	Assert(other_objp != NULL);

	dock_instance *result = dock_find_instance(objp, other_objp);
	
	if (result == NULL)
		return -1;
	else
		return result->dockpoint_used;
}

/**
 * Get the offset of the actual center of the docked ship models for the purposes of warping (which may not be the specified center).
 * Note, these are LOCAL coordinates in relation to objp, not world coordinates.
 * See also ship_class_get_actual_center() in ship.cpp
 */
void dock_calc_docked_actual_center(vec3d *dest, object *objp)
{
	Assert(dest != nullptr);
	Assert(objp != nullptr);

	vec3d overall_mins, overall_maxs;
	dock_calc_docked_extents(&overall_mins, &overall_maxs, objp);

	// c.f. ship_class_get_actual_center() in ship.cpp
	dest->xyz.x = (overall_maxs.xyz.x + overall_mins.xyz.x) * 0.5f;
	dest->xyz.y = (overall_maxs.xyz.y + overall_mins.xyz.y) * 0.5f;
	dest->xyz.z = (overall_maxs.xyz.z + overall_mins.xyz.z) * 0.5f;
}

/**
* Get the mins and maxs of the entire assembly of docked ship models.
* Note, these are LOCAL coordinates in relation to objp, not world coordinates.
*/
void dock_calc_docked_extents(vec3d *mins, vec3d *maxs, object *objp)
{
	Assert(mins != nullptr);
	Assert(maxs != nullptr);
	Assert(objp != nullptr);

	*mins = vmd_zero_vector;
	*maxs = vmd_zero_vector;

	// Let's calculate all mins/maxes in relation to the orientation of the main object
	// (which is expected to be the dock leader, but this technically isn't required).
	// Since the vast majority of dockpoints are aligned with an axis, this should
	// yield a much better fit of our docked bounding box.

	dock_function_info dfi;
	dfi.parameter_variables.objp_value = objp;		// the reference object for our bounding box orientation
	dfi.maintained_variables.vecp_value = mins;		// mins
	dfi.maintained_variables.vecp_value2 = maxs;	// maxs

	dock_evaluate_all_docked_objects(objp, &dfi, dock_calc_docked_mins_maxs_helper);
}

void dock_calc_docked_center_of_mass(vec3d *dest, object *objp)
{
	Assert(dest != NULL);
	Assert(objp != NULL);

	vm_vec_zero(dest);

	dock_function_info dfi;
	dfi.maintained_variables.vecp_value = dest;

	dock_evaluate_all_docked_objects(objp, &dfi, dock_calc_docked_center_of_mass_helper);

	// overall center of mass = weighted sum of centers of mass divided by total mass
	vm_vec_scale(dest, (1.0f / dfi.maintained_variables.float_value));
}

float dock_calc_total_docked_mass(object *objp)
{
	Assert(objp != NULL);

	dock_function_info dfi;
	
	dock_evaluate_all_docked_objects(objp, &dfi, dock_calc_total_docked_mass_helper);

	return dfi.maintained_variables.float_value;
}

float dock_calc_max_cross_sectional_radius_perpendicular_to_axis(object *objp, axis_type axis)
{
	Assert(objp != NULL);

	vec3d local_line_end;
	vec3d *world_line_start, world_line_end;
	dock_function_info dfi;

	// to calculate the cross-sectional radius, we need a line that will be perpendicular to the cross-section

	// the first endpoint is simply the position of the object
	world_line_start = &objp->pos;

	// the second endpoint extends in the axis direction
	vm_vec_zero(&local_line_end);
	switch(axis)
	{
		case X_AXIS:
			local_line_end.xyz.x = 1.0f;
			break;

		case Y_AXIS:
			local_line_end.xyz.y = 1.0f;
			break;

		case Z_AXIS:
			local_line_end.xyz.z = 1.0f;
			break;

		default:
			Int3();
			return 0.0f;
	}

	// rotate and move the endpoint to go through the axis of the actual object
	vm_vec_rotate(&world_line_end, &local_line_end, &objp->orient);
	vm_vec_add2(&world_line_end, &objp->pos);

	// now we have a unit vector starting at the object's position and pointing along the chosen axis
	// (although the length doesn't matter, as it's calculated as an endless line)

	// now determine the cross-sectional radius

	// set parameters and call function for the radius squared
	dfi.parameter_variables.vecp_value = world_line_start;
	dfi.parameter_variables.vecp_value2 = &world_line_end;
	dock_evaluate_all_docked_objects(objp, &dfi, dock_calc_max_cross_sectional_radius_squared_perpendicular_to_line_helper);

	// the radius is the square root of our result
	return fl_sqrt(dfi.maintained_variables.float_value);
}

float dock_calc_max_semilatus_rectum_parallel_to_axis(object *objp, axis_type axis)
{
	Assert(objp != NULL);

	vec3d local_line_end;
	vec3d *world_line_start, world_line_end;
	dock_function_info dfi;

	// to calculate the semilatus rectum, we need a directrix that will be parallel to the axis

	// the first endpoint is simply the position of the object
	world_line_start = &objp->pos;

	// the second endpoint extends in the axis direction
	vm_vec_zero(&local_line_end);
	switch(axis)
	{
		case X_AXIS:
			local_line_end.xyz.x = 1.0f;
			break;

		case Y_AXIS:
			local_line_end.xyz.y = 1.0f;
			break;

		case Z_AXIS:
			local_line_end.xyz.z = 1.0f;
			break;

		default:
			Int3();
			return 0.0f;
	}

	// rotate and move the endpoint to go through the axis of the actual object
	vm_vec_rotate(&world_line_end, &local_line_end, &objp->orient);
	vm_vec_add2(&world_line_end, &objp->pos);

	// now we have a unit vector starting at the object's position and pointing along the chosen axis
	// (although the length doesn't matter, as it's calculated as an endless line)

	// now determine the semilatus rectum

	// set parameters and call function for the semilatus rectum squared
	dfi.parameter_variables.vecp_value = world_line_start;
	dfi.parameter_variables.vecp_value2 = &world_line_end;
	dock_evaluate_all_docked_objects(objp, &dfi, dock_calc_max_semilatus_rectum_squared_parallel_to_directrix_helper);

	// the semilatus rectum is the square root of our result
	return fl_sqrt(dfi.maintained_variables.float_value);
}

float dock_calc_docked_fspeed(object *objp)
{
	Assert(objp != NULL);

	// *sigh*... the docked fspeed is simply the max fspeed of all docked objects
	dock_function_info dfi;
	dock_evaluate_all_docked_objects(objp, &dfi, dock_find_max_fspeed_helper);
	return dfi.maintained_variables.float_value;
}

float dock_calc_docked_speed(object *objp)
{
	Assert(objp != NULL);

	// ditto with speed
	dock_function_info dfi;
	dock_evaluate_all_docked_objects(objp, &dfi, dock_find_max_speed_helper);
	return dfi.maintained_variables.float_value;
}


// functions to deal with all docked ships anywhere
// ---------------------------------------------------------------------------------------------------------------

// universal two functions
// -----------------------

// evaluate a certain function for all docked objects
void dock_evaluate_all_docked_objects(object *objp, dock_function_info *infop, void (*function)(object *, dock_function_info *))
{
	Assert((objp != NULL) && (infop != NULL) && (function != NULL));

	// not docked?
	if (!object_is_docked(objp))
	{
		// call the function for just the one object
		function(objp, infop);
		return;
	}

	// we only have two objects docked
	if (dock_check_docked_one_on_one(objp))
	{
		// call the function for the first object, and return if instructed
		function(objp, infop);
		if (infop->early_return_condition) return;

		// call the function for the second object, and return if instructed
		function(objp->dock_list->docked_objp, infop);
		if (infop->early_return_condition) return;
	}

	// we have multiple objects docked and we're treating them as a hub
	else if (dock_check_assume_hub())
	{
		// get the hub
		object *hub_objp = dock_get_hub(objp);

		// call the function for the hub, and return if instructed
		function(hub_objp, infop);
		if (infop->early_return_condition) return;

		// iterate through all docked objects
		for (dock_instance *ptr = hub_objp->dock_list; ptr != NULL; ptr = ptr->next)
		{
			// call the function for this object, and return if instructed
			function(ptr->docked_objp, infop);
			if (infop->early_return_condition) return;
		}
	}

	// we have multiple objects docked and we must treat them as a tree
	else
	{
		// create a bit array to mark the objects we check
		ubyte *visited_bitstring = (ubyte *) vm_malloc(calculate_num_bytes(MAX_OBJECTS));

		// clear it
		memset(visited_bitstring, 0, calculate_num_bytes(MAX_OBJECTS));

		// start evaluating the tree
		dock_evaluate_tree(objp, infop, function, visited_bitstring);

		// destroy the bit array
		vm_free(visited_bitstring);
		visited_bitstring = NULL;
	}
}

void dock_evaluate_tree(object *objp, dock_function_info *infop, void (*function)(object *, dock_function_info *), ubyte *visited_bitstring)
{
	// make sure we haven't visited this object already
	if (get_bit(visited_bitstring, OBJ_INDEX(objp)))
		return;

	// mark as visited
	set_bit(visited_bitstring, OBJ_INDEX(objp));

	// call the function for this object, and return if instructed
	function(objp, infop);
	if (infop->early_return_condition) return;

	// iterate through all docked objects
	for (dock_instance *ptr = objp->dock_list; ptr != NULL; ptr = ptr->next)
	{
		// start another tree with the docked object as the root, and return if instructed
		dock_evaluate_tree(ptr->docked_objp, infop, function, visited_bitstring);
		if (infop->early_return_condition) return;
	}
}

// special-case functions
// ----------------------

void dock_move_docked_objects(object *objp)
{
	Assert(objp != NULL);

	if ((objp->type != OBJ_SHIP) && (objp->type != OBJ_START))
		return;

	if (!object_is_docked(objp))
		return;

	// has this object (by extension, this group of docked objects) been handled already?
	if (objp->flags[Object::Object_Flags::Docked_already_handled])
		return;

	Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));

	dock_function_info dfi;
	object *fastest_objp;

	// in FRED, objp is the object everyone moves with
	if (Fred_running)
	{
		fastest_objp = objp;
	}
	else
	{
		// find the object with the highest max speed
		dock_evaluate_all_docked_objects(objp, &dfi, dock_find_max_speed_helper);
		fastest_objp = dfi.maintained_variables.objp_value;

		// if we have no max speed, just use the first one
		if (fastest_objp == NULL)
			fastest_objp = objp;
	}
	
	// start a tree with that object as the parent... do NOT use the überfunction for this,
	// because we must use a tree for the parent ancestry to work correctly

	// we don't need a bit array because OF_DOCKED_ALREADY_HANDLED takes care of it
	// and must persist for the entire game frame

	// start evaluating the tree, starting with the fastest object having no parent
	dock_move_docked_children_tree(fastest_objp, NULL);
}

void dock_move_docked_children_tree(object *objp, object *parent_objp)
{
	// has this object been handled already?
	if (objp->flags[Object::Object_Flags::Docked_already_handled])
		return;

	// mark as handled
    objp->flags.set(Object::Object_Flags::Docked_already_handled);

	// if parent_objp exists
	if (parent_objp != NULL)
	{
		// move this object to align with it
		obj_move_one_docked_object(objp, parent_objp);
	}

	// iterate through all docked objects
	for (dock_instance *ptr = objp->dock_list; ptr != NULL; ptr = ptr->next)
	{
		// start another tree with the docked object as the root and this object as the parent
		dock_move_docked_children_tree(ptr->docked_objp, objp);
	}
}


// helper functions
// ----------------

void dock_count_total_docked_objects_helper(object * /*objp*/, dock_function_info *infop)
{
	// increment count
	infop->maintained_variables.int_value++;
}

void dock_check_find_docked_object_helper(object *objp, dock_function_info *infop)
{
	// if object found, set to true and break
	if (infop->parameter_variables.objp_value == objp)
	{
		infop->maintained_variables.bool_value = true;
		infop->early_return_condition = true;
	}
}

void dock_calc_docked_mins_maxs_helper(object *objp, dock_function_info *infop)
{
	polymodel *pm;
	vec3d parent_relative_mins, parent_relative_maxs;

	// find the model used by this object
	int modelnum = object_get_model(objp);
	Assert(modelnum >= 0);
	pm = model_get(modelnum);

	// special case: we are already in the correct frame of reference
	if (objp == infop->parameter_variables.objp_value)
	{
		parent_relative_mins = pm->mins;
		parent_relative_maxs = pm->maxs;
	}
	// we are not the parent object and need to do some gymnastics
	else
	{
		// get mins and maxs in world coordinates
		vec3d world_mins, world_maxs;
		vm_vec_unrotate(&world_mins, &pm->mins, &objp->orient);
		vm_vec_add2(&world_mins, &objp->pos);
		vm_vec_unrotate(&world_maxs, &pm->maxs, &objp->orient);
		vm_vec_add2(&world_maxs, &objp->pos);

		// now adjust them to be local to the parent
		vec3d temp_mins, temp_maxs;
		vm_vec_sub(&temp_mins, &world_mins, &infop->parameter_variables.objp_value->pos);
		vm_vec_rotate(&parent_relative_mins, &temp_mins, &infop->parameter_variables.objp_value->orient);
		vm_vec_sub(&temp_maxs, &world_maxs, &infop->parameter_variables.objp_value->pos);
		vm_vec_rotate(&parent_relative_maxs, &temp_maxs, &infop->parameter_variables.objp_value->orient);
	}

	// We test both points for both cases because they may have been flipped around.  However, X is still comparable to X, Y to Y, Z to Z.

	// test for overall min
	for (int i = 0; i < 3; ++i)
	{
		if (parent_relative_mins.a1d[i] < infop->maintained_variables.vecp_value->a1d[i])
			infop->maintained_variables.vecp_value->a1d[i] = parent_relative_mins.a1d[i];
		if (parent_relative_maxs.a1d[i] < infop->maintained_variables.vecp_value->a1d[i])
			infop->maintained_variables.vecp_value->a1d[i] = parent_relative_maxs.a1d[i];
	}

	// test for overall max
	for (int i = 0; i < 3; ++i)
	{
		if (parent_relative_mins.a1d[i] > infop->maintained_variables.vecp_value2->a1d[i])
			infop->maintained_variables.vecp_value2->a1d[i] = parent_relative_mins.a1d[i];
		if (parent_relative_maxs.a1d[i] > infop->maintained_variables.vecp_value2->a1d[i])
			infop->maintained_variables.vecp_value2->a1d[i] = parent_relative_maxs.a1d[i];
	}
}

void dock_calc_docked_center_of_mass_helper(object *objp, dock_function_info *infop)
{
	// add weighted object position and add mass
	vm_vec_scale_add2(infop->maintained_variables.vecp_value, &objp->pos, objp->phys_info.mass);
	infop->maintained_variables.float_value += objp->phys_info.mass;
}

void dock_calc_total_docked_mass_helper(object *objp, dock_function_info *infop)
{
	// add mass
	infop->maintained_variables.float_value += objp->phys_info.mass;
}

// What we're doing here is finding the distances between each extent of the object and the line, and then taking the
// maximum distance as the cross-sectional radius.  We're actually maintaining the square of the distance rather than
// the actual distance, as it's faster to calculate and it gives the same result in a greater-than or less-than
// comparison.  When we're done calculating everything for all objects (i.e. when we return to the parent function)
// we take the square root of the final value.
void dock_calc_max_cross_sectional_radius_squared_perpendicular_to_line_helper(object *objp, dock_function_info *infop)
{
	vec3d world_point, local_point[6], nearest;
	polymodel *pm;
	int i;
	float dist_squared;

	// line parameters
	vec3d *line_start = infop->parameter_variables.vecp_value;
	vec3d *line_end = infop->parameter_variables.vecp_value2;

	// We must find world coordinates for each of the six endpoints on the three axes of the object.  I looked up
	// which axis is front/back, left/right, and up/down, as well as which endpoint is which.  It doesn't really
	// matter, though, as all we need are the distances.

	// grab our model
	Assert(objp->type == OBJ_SHIP);
	pm = model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num);

	// set up the points we want to check
	memset(local_point, 0, sizeof(vec3d) * 6);
	local_point[0].xyz.x = pm->maxs.xyz.x;	// right point (max x)
	local_point[1].xyz.x = pm->mins.xyz.x;	// left point (min x)
	local_point[2].xyz.y = pm->maxs.xyz.y;	// top point (max y)
	local_point[3].xyz.y = pm->mins.xyz.y;	// bottom point (min y)
	local_point[4].xyz.z = pm->maxs.xyz.z;	// front point (max z)
	local_point[5].xyz.z = pm->mins.xyz.z;	// rear point (min z)

	// check points
	for (i = 0; i < 6; i++)
	{
		// calculate position of point
		vm_vec_rotate(&world_point, &local_point[i], &objp->orient);
		vm_vec_add2(&world_point, &objp->pos);

		// calculate square of distance to line
		vm_vec_dist_squared_to_line(&world_point, line_start, line_end, &nearest, &dist_squared);
	
		// update with farthest distance squared
		if (dist_squared > infop->maintained_variables.float_value)
			infop->maintained_variables.float_value = dist_squared;
	}
}

// What we're doing here is projecting each object extent onto the directrix, calculating the distance between the
// projected point and the origin, and then taking the maximum distance as the semilatus rectum.  We're actually
// maintaining the square of the distance rather than the actual distance, as it's faster to calculate and it gives
// the same result in a greater-than or less-than comparison.  When we're done calculating everything for all
// objects (i.e. when we return to the parent function) we take the square root of the final value.
void dock_calc_max_semilatus_rectum_squared_parallel_to_directrix_helper(object *objp, dock_function_info *infop)
{
	vec3d world_point, local_point[6], nearest;
	polymodel *pm;
	int i;
	float temp, dist_squared;

	// line parameters
	vec3d *line_start = infop->parameter_variables.vecp_value;
	vec3d *line_end = infop->parameter_variables.vecp_value2;

	// We must find world coordinates for each of the six endpoints on the three axes of the object.  I looked up
	// which axis is front/back, left/right, and up/down, as well as which endpoint is which.  It doesn't really
	// matter, though, as all we need are the distances.

	// grab our model
	Assert(objp->type == OBJ_SHIP);
	pm = model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num);

	// set up the points we want to check
	memset(local_point, 0, sizeof(vec3d) * 6);
	local_point[0].xyz.x = pm->maxs.xyz.x;	// right point (max x)
	local_point[1].xyz.x = pm->mins.xyz.x;	// left point (min x)
	local_point[2].xyz.y = pm->maxs.xyz.y;	// top point (max y)
	local_point[3].xyz.y = pm->mins.xyz.y;	// bottom point (min y)
	local_point[4].xyz.z = pm->maxs.xyz.z;	// front point (max z)
	local_point[5].xyz.z = pm->mins.xyz.z;	// rear point (min z)

	// check points
	for (i = 0; i < 6; i++)
	{
		// calculate position of point
		vm_vec_rotate(&world_point, &local_point[i], &objp->orient);
		vm_vec_add2(&world_point, &objp->pos);

		// find the nearest point along the line
		vm_vec_dist_squared_to_line(&world_point, line_start, line_end, &nearest, &temp);

		// find the distance squared between the origin of the line and the point on the line
		dist_squared = vm_vec_dist_squared(line_start, &nearest);
	
		// update with farthest distance squared
		if (dist_squared > infop->maintained_variables.float_value)
			infop->maintained_variables.float_value = dist_squared;
	}
}

void dock_find_max_fspeed_helper(object *objp, dock_function_info *infop)
{
	// check our fspeed against the running maximum
	if (objp->phys_info.fspeed > infop->maintained_variables.float_value)
	{
		infop->maintained_variables.float_value = objp->phys_info.fspeed;
		infop->maintained_variables.objp_value = objp;
	}
}

void dock_find_max_speed_helper(object *objp, dock_function_info *infop)
{
	// check our speed against the running maximum
	if (objp->phys_info.speed > infop->maintained_variables.float_value)
	{
		infop->maintained_variables.float_value = objp->phys_info.speed;
		infop->maintained_variables.objp_value = objp;
	}
}
// ---------------------------------------------------------------------------------------------------------------
// end of über code block ----------------------------------------------------------------------------------------

// dock management functions -------------------------------------------------------------------------------------
void dock_dock_objects(object *objp1, int dockpoint1, object *objp2, int dockpoint2)
{
	Assert(objp1 != NULL);
	Assert(objp2 != NULL);

#ifndef NDEBUG
	if ((dock_find_instance(objp1, objp2) != NULL) || (dock_find_instance(objp2, objp1) != NULL))
	{
		Error(LOCATION, "Trying to dock an object that's already docked!\n");
	}

	if ((dock_find_instance(objp1, dockpoint1) != NULL) || (dock_find_instance(objp2, dockpoint2) != NULL))
	{
		Error(LOCATION, "Trying to dock to a dockpoint that's in use!\n");
	}
#endif

	// put objects on each others' dock lists 
	dock_add_instance(objp1, dockpoint1, objp2);
	dock_add_instance(objp2, dockpoint2, objp1);
}

void dock_undock_objects(object *objp1, object *objp2)
{
	Assert(objp1 != NULL);
	Assert(objp2 != NULL);

	// remove objects from each others' dock lists
	dock_remove_instance(objp1, objp2);
	dock_remove_instance(objp2, objp1);
}

void dock_undock_all(object *objp)
{
	Assert(objp != NULL);

	while (object_is_docked(objp))
	{
		object* dockee = dock_get_first_docked_object(objp);

		dock_undock_objects(objp, dockee);
	}
}

// dock list functions -------------------------------------------------------------------------------------------
bool dock_check_assume_hub()
{
	// There are several ways of handling ships docking to other ships.  Level 1, the simplest, is the one-docker, one-dockee
	// model used in retail FS2.  Level 2 is the hub model, where we stipulate that any given set of docked ships
	// includes one ship to which all other ships are docked.  No ship except for the hub ship can be docked to more than
	// one ship.  Level 3 is the daisy-chain model, where you can string ships along and make a rooted tree.
	//
	// The new code can handle level 3 ship formations, but it requires more overhead than level 2 or level 1.  (Whether
	// the additional overhead is significant or not has not been determined.)  In the vast majority of cases, level 3
	// is not needed.  So this function is provided to allow the code to optimize itself for level 2, should level 1
	// evaluation fail.

	// Assume level 2 optimization unless the mission specifies level 3.
	return !(The_mission.flags[Mission::Mission_Flags::Allow_dock_trees]);
}

object *dock_get_hub(object *objp)
{
	Assert(dock_check_assume_hub() && object_is_docked(objp));

	// if our dock list contains only one object, it must be the hub
	if (objp->dock_list->next == NULL)
	{
		return dock_get_first_docked_object(objp);
	}
	// otherwise we are the hub
	else
	{
		return objp;
	}
}

void dock_add_instance(object *objp, int dockpoint, object *other_objp)
{
	dock_instance *item;

	// create item
	item = (dock_instance *) vm_malloc(sizeof(dock_instance));
	item->dockpoint_used = dockpoint;
	item->docked_objp = other_objp;

	// prepend item to existing list
	item->next = objp->dock_list;
	objp->dock_list = item;
}

void dock_remove_instance(object *objp, object *other_objp)
{
	int found = 0;
	dock_instance *prev_ptr, *ptr;
	
	prev_ptr = NULL;
	ptr = objp->dock_list;

	// iterate until item found
	while (ptr != NULL)
	{
		// if found, exit loop
		if (ptr->docked_objp == other_objp)
		{
			found = 1;
			break;
		}

		// iterate
		prev_ptr = ptr;
		ptr = ptr->next;
	}

	// delete if found
	if (found)
	{
		// special case... found at beginning of list
		if (prev_ptr == NULL)
		{
			objp->dock_list = ptr->next;
		}
		// normal case
		else
		{
			prev_ptr->next = ptr->next;
		}

		// delete it
		vm_free(ptr);
	}
	else
	{
		// Trigger an assertion, we can recover from this one, thankfully.
		UNREACHABLE("Tried to undock an object that isn't docked!\n");
	}
}

// just free the list without worrying about undocking anything
void dock_free_dock_list(object *objp)
{
	Assert(objp != NULL);

	while (objp->dock_list != NULL)
	{
		dock_instance *ptr = objp->dock_list;
		objp->dock_list = ptr->next;
		vm_free(ptr);
	}
}

dock_instance *dock_find_instance(object *objp, object *other_objp)
{
	dock_instance *ptr = objp->dock_list;

	// iterate until item found
	while (ptr != NULL)
	{
		// if found, return it
		if (ptr->docked_objp == other_objp)
			return ptr;

		// iterate
		ptr = ptr->next;
	}

	// not found
	return NULL;
}

dock_instance *dock_find_instance(object *objp, int dockpoint)
{
	dock_instance *ptr = objp->dock_list;

	// iterate until item found
	while (ptr != NULL)
	{
		// if found, return it
		if (ptr->dockpoint_used == dockpoint)
			return ptr;

		// iterate
		ptr = ptr->next;
	}

	// not found
	return NULL;
}

int dock_count_instances(object *objp)
{
	int total_count = 0;

	// count all instances in the list
	dock_instance *ptr = objp->dock_list;
	while (ptr != NULL)
	{
		// incrememnt for this object
		total_count++;

		// iterate
		ptr = ptr->next;
	}

	// done
	return total_count;
}
