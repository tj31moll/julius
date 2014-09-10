#include "Resource.h"

#include "Calc.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Model.h"
#include "Data/Scenario.h"

static struct {
	int buildingIds[100];
	int numItems;
	int totalStorageWheat;
	int totalStorageVegetables;
	int totalStorageFruit;
	int totalStorageMeat;
} nonGettingGranaries;

static int isFood(int resource) {
	switch (resource) {
		case Resource_Wheat:
		case Resource_Vegetables:
		case Resource_Fruit:
		case Resource_Meat:
			return 1;
		default:
			return 0;
	}
}

void Resource_gatherGranaryGettingInfo()
{
	nonGettingGranaries.numItems = 0;
	for (int i = 0; i < 100; i++) {
		nonGettingGranaries.buildingIds[i] = 0;
	}
	nonGettingGranaries.totalStorageWheat = 0;
	nonGettingGranaries.totalStorageVegetables = 0;
	nonGettingGranaries.totalStorageFruit = 0;
	nonGettingGranaries.totalStorageMeat = 0;

	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (b->inUse != 1 || b->type != Building_Granary) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0) {
			continue;
		}
		struct Data_Building_Storage *s = &Data_Building_Storages[b->storageId];
		int totalNonGetting = 0;
		if (s->resourceState[Resource_Wheat] != BuildingStorageState_Getting) {
			totalNonGetting += b->data.storage.resourceStored[Resource_Wheat];
			nonGettingGranaries.totalStorageWheat += b->data.storage.resourceStored[Resource_Wheat];
		}
		if (s->resourceState[Resource_Vegetables] != BuildingStorageState_Getting) {
			totalNonGetting += b->data.storage.resourceStored[Resource_Vegetables];
			nonGettingGranaries.totalStorageVegetables += b->data.storage.resourceStored[Resource_Vegetables];
		}
		if (s->resourceState[Resource_Fruit] != BuildingStorageState_Getting) {
			totalNonGetting += b->data.storage.resourceStored[Resource_Fruit];
			nonGettingGranaries.totalStorageFruit += b->data.storage.resourceStored[Resource_Fruit];
		}
		if (s->resourceState[Resource_Meat] != BuildingStorageState_Getting) {
			totalNonGetting += b->data.storage.resourceStored[Resource_Meat];
			nonGettingGranaries.totalStorageMeat += b->data.storage.resourceStored[Resource_Meat];
		}
		if (totalNonGetting > 100) {
			nonGettingGranaries.buildingIds[nonGettingGranaries.numItems] = i;
			if (nonGettingGranaries.numItems < 98) {
				nonGettingGranaries.numItems++;
			}
		}
	}
}

int Resource_getGranaryForStoringFood(
	int forceOnStockpile, int x, int y, int resource, int distanceFromEntry, int roadNetworkId,
	int *understaffed, int *xDst, int *yDst)
{
	if (Data_Scenario.romeSuppliesWheat) {
		return 0;
	}
	if (!isFood(resource)) {
		return 0;
	}
	if (Data_CityInfo.resourceStockpiled[resource] && !forceOnStockpile) {
		return 0;
	}
	int minDist = 10000;
	int minBuildingId = 0;
	for (int i = 0; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (b->inUse != 1 || b->type != Building_Granary) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0 || b->roadNetworkId != roadNetworkId) {
			continue;
		}
		int pctWorkers = Calc_getPercentage(b->numWorkers,
			Data_Model_Buildings[b->type].laborers);
		if (pctWorkers < 100) {
			if (understaffed) {
				*understaffed += 1;
			}
			continue;
		}
		struct Data_Building_Storage *s = &Data_Building_Storages[b->storageId];
		if (s->resourceState[resource] == BuildingStorageState_NotAccepting || s->emptyAll) {
			continue;
		}
		if (b->data.storage.resourceStored[Resource_None] >= 100) {
			// there is room
			int dist = Resource_getDistance(b->x, b->y, x, y, distanceFromEntry, b->distanceFromEntry);
			if (dist < minDist) {
				minDist = dist;
				minBuildingId = i;
			}
		}
	}
	// deliver to center of granary
	*xDst = Data_Buildings[minBuildingId].x + 1;
	*yDst = Data_Buildings[minBuildingId].y + 1;
	return minBuildingId;
}

int Resource_getGettingGranaryForStoringFood(
	int x, int y, int resource, int distanceFromEntry, int roadNetworkId,
	int *xDst, int *yDst)
{
	if (Data_Scenario.romeSuppliesWheat) {
		return 0;
	}
	if (!isFood(resource)) {
		return 0;
	}
	if (Data_CityInfo.resourceStockpiled[resource]) {
		return 0;
	}
	int minDist = 10000;
	int minBuildingId = 0;
	for (int i = 0; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (b->inUse != 1 || b->type != Building_Granary) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0 || b->roadNetworkId != roadNetworkId) {
			continue;
		}
		int pctWorkers = Calc_getPercentage(b->numWorkers,
			Data_Model_Buildings[b->type].laborers);
		if (pctWorkers < 100) {
			continue;
		}
		struct Data_Building_Storage *s = &Data_Building_Storages[b->storageId];
		if (s->resourceState[resource] != BuildingStorageState_Getting || s->emptyAll) {
			continue;
		}
		if (b->data.storage.resourceStored[Resource_None] > 100) {
			// there is room
			int dist = Resource_getDistance(b->x, b->y, x, y, distanceFromEntry, b->distanceFromEntry);
			if (dist < minDist) {
				minDist = dist;
				minBuildingId = i;
			}
		}
	}
	*xDst = Data_Buildings[minBuildingId].x;
	*yDst = Data_Buildings[minBuildingId].y;
	return minBuildingId;
}

int Resource_getGranaryForGettingFood(int srcBuildingId, int *xDst, int *yDst)
{
	struct Data_Building *src = &Data_Buildings[srcBuildingId];
	struct Data_Building_Storage *srcStorage = &Data_Building_Storages[src->storageId];
	if (srcStorage->emptyAll) {
		return 0;
	}
	if (Data_Scenario.romeSuppliesWheat) {
		return 0;
	}
	int numGetting = 0;
	if (srcStorage->resourceState[Resource_Wheat] == BuildingStorageState_Getting) {
		numGetting++;
	}
	if (srcStorage->resourceState[Resource_Vegetables] == BuildingStorageState_Getting) {
		numGetting++;
	}
	if (srcStorage->resourceState[Resource_Fruit] == BuildingStorageState_Getting) {
		numGetting++;
	}
	if (srcStorage->resourceState[Resource_Meat] == BuildingStorageState_Getting) {
		numGetting++;
	}
	if (numGetting <= 0) {
		return 0;
	}

	int minDist = 10000;
	int minBuildingId = 0;
	for (int i = 0; i < nonGettingGranaries.numItems; i++) {
		int buildingId = nonGettingGranaries.buildingIds[i];
		struct Data_Building *b = &Data_Buildings[buildingId];
		if (b->roadNetworkId != src->roadNetworkId) {
			continue;
		}
		struct Data_Building_Storage *s = &Data_Building_Storages[b->storageId];
		int amountGettable = 0;
		if (srcStorage->resourceState[Resource_Wheat] == BuildingStorageState_Getting &&
			s->resourceState[Resource_Wheat] != BuildingStorageState_Getting) {
			amountGettable += b->data.storage.resourceStored[Resource_Wheat];
		}
		if (srcStorage->resourceState[Resource_Vegetables] == BuildingStorageState_Getting &&
			s->resourceState[Resource_Vegetables] != BuildingStorageState_Getting) {
			amountGettable += b->data.storage.resourceStored[Resource_Vegetables];
		}
		if (srcStorage->resourceState[Resource_Fruit] == BuildingStorageState_Getting &&
			s->resourceState[Resource_Fruit] != BuildingStorageState_Getting) {
			amountGettable += b->data.storage.resourceStored[Resource_Fruit];
		}
		if (srcStorage->resourceState[Resource_Meat] == BuildingStorageState_Getting &&
			s->resourceState[Resource_Meat] != BuildingStorageState_Getting) {
			amountGettable += b->data.storage.resourceStored[Resource_Meat];
		}
		if (amountGettable > 0) {
			int dist = Resource_getDistance(b->x, b->y, src->x, src->y, src->distanceFromEntry, b->distanceFromEntry);
			if (amountGettable <= 400) {
				dist *= 2; // penalty for less food
			}
			if (dist < minDist) {
				minDist = dist;
				minBuildingId = buildingId;
			}
		}
	}
	*xDst = Data_Buildings[minBuildingId].x;
	*yDst = Data_Buildings[minBuildingId].y;
	return minBuildingId;
}

int Resource_getAmountStoredInGranary(int buildingId, int resource)
{
	if (buildingId <= 0) {
		return 0;
	}
	if (!isFood(resource)) {
		return 0;
	}
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->type != Building_Granary) {
		return 0;
	}
	return b->data.storage.resourceStored[resource];
}

int Resource_addToGranary(int buildingId, int resource, int countAsProduced)
{
	if (buildingId <= 0) {
		return 1;
	}
	if (!isFood(resource)) {
		return 0;
	}
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->type != Building_Granary) {
		return 0;
	}
	if (b->data.storage.resourceStored[Resource_None] <= 0) {
		return 0;
	}
	if (countAsProduced) {
		Data_CityInfo.foodInfoFoodStoredSoFarThisMonth += 100;
	}
	if (b->data.storage.resourceStored[Resource_None] <= 100) {
		b->data.storage.resourceStored[resource] += b->data.storage.resourceStored[Resource_None];
		b->data.storage.resourceStored[Resource_None] = 0;
	} else {
		b->data.storage.resourceStored[resource] += 100;
		b->data.storage.resourceStored[Resource_None] -= 100;
	}
	return 1;
}

int Resource_removeFromGranary(int buildingId, int resource, int amount)
{
	if (amount <= 0) {
		return 0;
	}
	int toRemove;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->data.storage.resourceStored[resource] >= amount) {
		Data_CityInfo.resourceGranaryFoodStored[resource] -= amount;
		b->data.storage.resourceStored[resource] -= amount;
		b->data.storage.resourceStored[Resource_None] += amount;
		toRemove = 0;
	} else {
		int removed = b->data.storage.resourceStored[resource];
		Data_CityInfo.resourceGranaryFoodStored[resource] -= removed;
		b->data.storage.resourceStored[resource] -= removed;
		b->data.storage.resourceStored[Resource_None] += removed;
		toRemove = amount - removed;
	}
	return toRemove;
}

int Resource_determineGranaryWorkerTask(int buildingId)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	int pctWorkers = Calc_getPercentage(b->numWorkers, Data_Model_Buildings[b->type].laborers);
	if (pctWorkers < 50) {
		return -1;
	}
	struct Data_Building_Storage *s = &Data_Building_Storages[b->storageId];
	if (s->emptyAll) {
		// bring food to another granary
		for (int i = Resource_Wheat; i <= Resource_Meat; i++) {
			if (b->data.storage.resourceStored[i]) {
				return i;
			}
		}
		return -1;
	}
	if (b->data.storage.resourceStored[Resource_None] <= 0) {
		return -1; // nothing to get
	}
	if (s->resourceState[Resource_Wheat] == BuildingStorageState_Getting && nonGettingGranaries.totalStorageFruit > 100) {
		return 0;
	}
	if (s->resourceState[Resource_Vegetables] == BuildingStorageState_Getting && nonGettingGranaries.totalStorageWheat > 100) {
		return 0;
	}
	if (s->resourceState[Resource_Fruit] == BuildingStorageState_Getting && nonGettingGranaries.totalStorageWheat > 100) {
		return 0;
	}
	if (s->resourceState[Resource_Meat] == BuildingStorageState_Getting && nonGettingGranaries.totalStorageWheat > 100) {
		return 0;
	}
	return -1;
}

int Resource_takeFoodFromGranaryForGettingDeliveryman(int dstBuildingId, int srcBuildingId, int *resource)
{
	struct Data_Building *bSrc = &Data_Buildings[srcBuildingId];
	struct Data_Building_Storage *sSrc = &Data_Building_Storages[bSrc->storageId];
	struct Data_Building *bDst = &Data_Buildings[dstBuildingId];
	struct Data_Building_Storage *sDst = &Data_Building_Storages[bDst->storageId];
	
	int maxAmount = 0;
	int maxResource = 0;
	if (sDst->resourceState[Resource_Wheat] == BuildingStorageState_Getting &&
		sSrc->resourceState[Resource_Wheat] != BuildingStorageState_Getting) {
		if (bSrc->data.storage.resourceStored[Resource_Wheat] > maxAmount) {
			maxAmount = bSrc->data.storage.resourceStored[Resource_Wheat];
			maxResource = Resource_Wheat;
		}
	}
	if (sDst->resourceState[Resource_Vegetables] == BuildingStorageState_Getting &&
		sSrc->resourceState[Resource_Vegetables] != BuildingStorageState_Getting) {
		if (bSrc->data.storage.resourceStored[Resource_Vegetables] > maxAmount) {
			maxAmount = bSrc->data.storage.resourceStored[Resource_Vegetables];
			maxResource = Resource_Vegetables;
		}
	}
	if (sDst->resourceState[Resource_Fruit] == BuildingStorageState_Getting &&
		sSrc->resourceState[Resource_Fruit] != BuildingStorageState_Getting) {
		if (bSrc->data.storage.resourceStored[Resource_Fruit] > maxAmount) {
			maxAmount = bSrc->data.storage.resourceStored[Resource_Fruit];
			maxResource = Resource_Fruit;
		}
	}
	if (sDst->resourceState[Resource_Meat] == BuildingStorageState_Getting &&
		sSrc->resourceState[Resource_Meat] != BuildingStorageState_Getting) {
		if (bSrc->data.storage.resourceStored[Resource_Meat] > maxAmount) {
			maxAmount = bSrc->data.storage.resourceStored[Resource_Meat];
			maxResource = Resource_Meat;
		}
	}
	
	if (maxAmount > 800) {
		maxAmount = 800;
	}
	if (maxAmount > bDst->data.storage.resourceStored[Resource_None]) {
		maxAmount = bDst->data.storage.resourceStored[Resource_None];
	}
	Resource_removeFromGranary(srcBuildingId, maxResource, maxAmount);
	*resource = maxResource;
	return maxAmount / 100;
}
