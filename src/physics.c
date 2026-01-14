#include "physics.h"
#include <math.h>

// check if a point is inside a block's bounding box (AABB collision)
// returns the Y position of the top of the block if collision, or -INFINITY if no collision
float checkBlockCollisionY(Vector3 pos, float footY, DataObj* block){
	if(block->asInt[0] == 0) return -INFINITY; // collision disabled
	
	if(block->asInt[1] != 1) return -INFINITY; // not a solid block
	
	// block bounds (pos is corner, scale is size)
	float bMinX = block->pos.x;
	float bMaxX = block->pos.x + block->scale.x;
	float bMinY = block->pos.y;
	float bMaxY = block->pos.y + block->scale.y;
	float bMinZ = block->pos.z;
	float bMaxZ = block->pos.z + block->scale.z;
	
	if(pos.x >= bMinX && pos.x <= bMaxX &&
	   pos.z >= bMinZ && pos.z <= bMaxZ){
		if(footY <= bMaxY && footY >= bMinY - 0.5f){
			return bMaxY;
		}
	}
	return -INFINITY;
}

float findFloorY(Vector3 pos, float footY, DataObj* item){
	float highestFloor = -INFINITY;
	
	float blockFloor = checkBlockCollisionY(pos, footY, item);
	if(blockFloor > highestFloor) highestFloor = blockFloor;
	
	DataObj* child = item->child;
	while(child){
		float childFloor = findFloorY(pos, footY, child);
		if(childFloor > highestFloor) highestFloor = childFloor;
		child = child->next;
	}
	
	return highestFloor;
}