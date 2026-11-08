#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "physics.h"
#include "math.h"

#include "instances.h"

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
			lesserBetween(itemA->pos.x - itemB->pos.x, -itemA->scale.x, itemB->scale.x) && 
			lesserBetween(itemB->pos.y - itemA->pos.y, -itemA->scale.y, itemB->scale.y) && 
			lesserBetween(itemA->pos.z - itemB->pos.z, -itemA->scale.z, itemB->scale.z)
		)) 
			return NULL;
		
		output = malloc(sizeof(CollisionReturn));
		output->hullA = itemA; output->hullB = itemB;

		float distList[6] = {
/*MINDIST_PX*/	fabs(itemB->pos.x - itemA->pos.x - itemA->scale.x),
/*MINDIST_NX*/	fabs((itemB->pos.x + itemB->scale.x) - itemA->pos.x),
/*MINDIST_PY*/	fabs(itemB->pos.y - itemA->pos.y + itemA->scale.y) - 1,
/*MINDIST_NY*/	fabs((itemB->pos.y - itemB->scale.y) - itemA->pos.y),
/*MINDIST_PZ*/	fabs(itemB->pos.z - itemA->pos.z - itemA->scale.z),
/*MINDIST_NZ*/	fabs((itemB->pos.z + itemB->scale.z) - itemA->pos.z),
		};

		Uint16 smallestIndex = minIndex(distList, 6);
		//char* smallIndex = malloc(64); sprintf(smallIndex, "Smallest Index: %d (%f)", smallestIndex, distList[smallestIndex]);
		//sendPopup(smallIndex, NULL, NULL, 0.06);
		switch(smallestIndex){
			case MINDIST_PX: output->outNorm = (Vector3){itemB->pos.x - itemA->pos.x - itemA->scale.x, 0, 0}; break;
			case MINDIST_NX: output->outNorm = (Vector3){(itemB->pos.x + itemB->scale.x) - itemA->pos.x, 0, 0}; break;

			case MINDIST_PY: output->outNorm = (Vector3){0, itemB->pos.y - itemA->pos.y + itemA->scale.y, 0}; break;
			case MINDIST_NY: output->outNorm = (Vector3){0, (itemB->pos.y - itemB->scale.y) - itemA->pos.y, 0}; break;

			case MINDIST_PZ: output->outNorm = (Vector3){0, 0, itemB->pos.z - itemA->pos.z - itemA->scale.z}; break;
			case MINDIST_NZ: output->outNorm = (Vector3){0, 0, (itemB->pos.z + itemB->scale.z) - itemA->pos.z}; break;

			default: output->outNorm = (Vector3){0, itemB->pos.y - itemA->pos.y + itemA->scale.y, 0}; break;
		}
		
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
	if(!(coll->hullA || coll->hullB)) return;

	bool bothFix = (coll->hullA->active && coll->hullB->active);
	(void)bothFix;

	DataObj* itemA = coll->hullA->parent;
	DataObj* itemB = coll->hullB->parent;
	if(!(itemA && itemB)) return;

	DataObj* currItem;
	if(!bothFix){
		currItem = coll->hullA->active ? itemA : itemB;
		Vector3 newVec = vec3Add(currItem->pos, coll->outNorm);
		currItem->pos = newVec;
		return;
	}else{
		Vector3 halfNorm = vec3Mult(coll->outNorm, (Vector3){0.5, 0.5, 0.5}); //make change based on mass or something
		itemA->pos = vec3Add(itemA->pos, halfNorm);
		itemB->pos = vec3Sub(itemB->pos, halfNorm);
	}

	//printf("Resolved collision between %s and %s with an intersect normal of %f, %f, %f\n", itemA->name, itemB->name, coll->outNorm.x, coll->outNorm.y, coll->outNorm.z);
}

extern DataType playerClass;
Vector3 lazyCollisionLoop(DataObj* object, DataObj* item){
	if(!object->objColl)
		return (Vector3){0, 0, 0};
	Vector3 totalOut = {0, 0, 0};
	DataObj* child = item->child;
	while(child){
		if(!child->objColl || child == object) goto collideLoopSkip;

		CollisionReturn* collision = getCollision(object->objColl, child->objColl);
		if(collision){
			resolveCollision(collision);
			totalOut = vec3Add(totalOut, collision->outNorm);
			free(collision);
		}
collideLoopSkip:
		totalOut = vec3Add(totalOut, lazyCollisionLoop(object, child));
		child = child->next;
	}
	return totalOut;
}

RayCast* castRay(Vector3 pos, Vector3 rot){
	RayCast* newRay = malloc(sizeof(RayCast));
	if(!newRay) return NULL;

	newRay->pos = pos;
	newRay->norm = rotToNorm3(rot);

	return newRay;
}

Vector3 rayDistPos(RayCast* ray, float dist){
	return vec3Add(ray->pos, vec3Mult(ray->norm, (Vector3){dist, dist, dist}));
}

RayReturn* rayCubeCheck(RayCast* ray, CollisionHull* hull){
	if(!ray || !hull) return NULL;
	if(hull->shape != COLLHULL_CUBE) return NULL;

	Vector3 minPos = hull->pos;
	Vector3 maxPos = vec3Add(hull->pos, hull->scale);

	float xt0 = (minPos.x - ray->pos.x) / ray->norm.x;
	float xt1 = (maxPos.x - ray->pos.x) / ray->norm.x;

	if(xt1 < xt0){
		float temp = xt0;
		xt0 = xt1;
		xt1 = temp;
	}

	float yt0 = (minPos.y - ray->pos.y) / ray->norm.y;
	float yt1 = (maxPos.y - ray->pos.y) / ray->norm.y;

	if(yt1 < yt0){
		float temp = yt0;
		yt0 = yt1;
		yt1 = temp;
	}

	if(xt0 > yt1 || yt0 > xt1) return NULL;

	float t0 = yt0 > xt0 ? yt0 : xt0;
	float t1 = yt1 < xt1 ? yt1 : xt1;

	float zt0 = (minPos.z - ray->pos.z) / ray->norm.z;
	float zt1 = (maxPos.z - ray->pos.z) / ray->norm.z;

	if(zt1 < zt0){
		float temp = zt0;
		zt0 = zt1;
		zt1 = temp;
	}

	if(t0 > zt1 || zt0 > t1) return NULL;

	t0 = zt0 > t0 ? zt0 : t0;
	t1 = zt1 < t1 ? zt1 : t1;

	RayReturn* rayHit = malloc(sizeof(RayReturn));
	if(!rayHit) return NULL;

	rayHit->dist = t0; rayHit->depth = t1 - t0;
	rayHit->hitPos = rayDistPos(ray, t0);
	rayHit->hit = hull;

	return rayHit;
}

RayReturn* getRayIntersect(RayCast* ray, RayReturn* oldHit, DataObj* item){
	if(!ray) return NULL;

	RayReturn* closestRay = oldHit;
	DataObj* child = item->child;
	while(child){
		if(!child->objColl) goto rayCheckSkip;
		RayReturn* rayHit = rayCubeCheck(ray, child->objColl);
		if(!rayHit){
			//printf("Ray didnt hit %s\n", child->name);
			goto rayCheckSkip;
		}
		
		if(!closestRay){
			closestRay = rayHit;
			goto rayCheckSkip;
		}

		if(fabs(rayHit->dist) < closestRay->dist){
			//printf("Closest ray: %f %f %f\n", closestRay->hitPos.x, closestRay->hitPos.y, closestRay->hitPos.z);
			free(closestRay);
			closestRay = rayHit;
		}else{
			free(rayHit);
		}

rayCheckSkip:
		RayReturn* newRay = getRayIntersect(ray, closestRay, child);
		closestRay = newRay ? newRay : closestRay;
		child = child->next;
	}
	return closestRay;
}