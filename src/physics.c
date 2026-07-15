#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "physics.h"
#include "math.h"

// check if a point is inside a block's bounding box (AABB collision)
// returns the Y position of the top of the block if collision, or -INFINITY if no collision
float checkBlockCollisionY(Vector3 pos, float footY, DataObj* block){
	CollisionHull *collider = block->objColl;
	
	if(!collider) return -INFINITY; // collision disabled
	
	if(collider->shape != COLLHULL_CUBE) return -INFINITY; // not a solid block
	
	// block bounds (pos is corner, scale is size)
	Vector3 bMin = {block->pos.x - 0.5, block->pos.y - block->scale.y - 4, block->pos.z - 0.5};
	Vector3 bMax = {block->pos.x + block->scale.x + 0.5, block->pos.y, block->pos.z + block->scale.z + 0.5};
	
	if(pos.x >= bMin.x && pos.x <= bMax.x &&
	   pos.z >= bMin.z && pos.z <= bMax.z){
		if(footY <= bMax.y && footY >= bMin.y - 0.5f)
			return bMax.y;
	}
	return -INFINITY;
}

extern ClientData client;
float checkSphereCollisionY(Vector3 pos, DataObj* block){
	CollisionHull *collider = block->objColl;
	
	if(!collider) return -INFINITY; // collision disabled
	
	if(collider->shape != COLLHULL_SPHERE) return -INFINITY; // not a solid block
	
	// block bounds (pos is corner, scale is size)
	float bRadius = (block->scale.x + block->scale.y + block->scale.z) / 6;
	Vector3 bDist = {block->pos.x + block->scale.x / 2 - pos.x, block->pos.y - block->scale.y / 2  - pos.y, block->pos.z + block->scale.z / 2  - pos.z};
	float pythag = bDist.x * bDist.x + bDist.y * bDist.y + bDist.z * bDist.z;
	
	if(pythag <= bRadius * bRadius){
		//return block->pos.y + vec3Mult(normalize3(block->pos), (Vector3){block->scale.x / 2, block->scale.y / 2, block->scale.z / 2}).y;
		return block->pos.y - (fabs(SDL_sin(bDist.x / block->scale.x)) + fabs(SDL_sin(bDist.z / block->scale.z))) * (block->scale.y / 2);
	}
	return -INFINITY;
}

float findFloorY(Vector3 pos, float footY, DataObj* item){
	float highestFloor = -INFINITY;
	
	float blockFloor = max(checkBlockCollisionY(pos, footY, item), checkSphereCollisionY(pos, item));
	highestFloor = max(blockFloor, highestFloor);
	
	if(item == client.gameWorld->currPlayer) return -INFINITY;
	DataObj* child = item->child;
	while(child){
		float childFloor = findFloorY(pos, footY, child);
		if(childFloor > highestFloor) highestFloor = childFloor;
		child = child->next;
	}
	
	return highestFloor;
}

CollisionReturn* getCollision(CollisionHull* itemA, CollisionHull* itemB){
	CollisionReturn *output = NULL;
	
	if(itemA->shape == COLLHULL_CUBE && itemB->shape == COLLHULL_CUBE){
		if(!(
			between(itemA->pos.x - itemB->pos.x, -itemA->scale.x, itemB->scale.x) && 
			between(itemB->pos.y - itemA->pos.y, -itemA->scale.y, itemB->scale.y) && 
			between(itemA->pos.z - itemB->pos.z, -itemA->scale.z, itemB->scale.z)
		)) 
			return NULL;
		
		output = malloc(sizeof(CollisionReturn));
		output->outNorm = (Vector3){0, itemB->pos.y - itemA->pos.y + itemA->scale.y, 0};
		output->hullA = itemA; output->hullB = itemB;
		return output;
	}

	if(itemA->shape == COLLHULL_SPHERE && itemB->shape == COLLHULL_SPHERE){
		output = malloc(sizeof(CollisionReturn));
		output->outNorm = (Vector3){0, itemB->pos.y - itemA->pos.y, 0};
		output->hullA = itemA; output->hullB = itemB;
		return output;
	}

	//if(!output)
	//	printf("Couldnt find a collision between %s and %s\n", itemA->parent->name, itemB->parent->name);
	
	/*if(collide is yes) then do'eth
		tell me the collision outputs then please
	  else*/
	return NULL;
	//end
}

void resolveCollision(CollisionReturn* coll){
	if(!coll->hullA->active && !coll->hullB->active) return;

	bool bothFix = (!coll->hullA->active && !coll->hullB->active) && (coll->hullA->active != coll->hullB->active);
	(void)bothFix;

	DataObj* itemA = coll->hullA->parent;
	DataObj* itemB = coll->hullB->parent;
	if(!itemA || !itemB) return;
	itemA->pos = (Vector3){itemA->pos.x + coll->outNorm.x, itemA->pos.y + coll->outNorm.y, itemA->pos.z + coll->outNorm.z};

	//printf("Resolved collision between %s and %s with an intersect normal of %f, %f, %f\n", itemA->name, itemB->name, coll->outNorm.x, coll->outNorm.y, coll->outNorm.z);
}

extern DataType playerClass;
bool lazyCollisionLoop(DataObj* object, DataObj* item){
	if(!object->objColl)
		return false;
	bool collided = false;
	DataObj* child = item->child;
	while(child){
		if(!child->objColl || child == object) goto collideLoopSkip;

		CollisionReturn* collision = getCollision(object->objColl, child->objColl);
		if(collision){
			resolveCollision(collision);
			free(collision);

			collided = true;
		}
collideLoopSkip:
		collided = lazyCollisionLoop(object, child) || collided;
		child = child->next;
	}
	return collided;
}