#include <engine.hpp>
#include <lightEngine.hpp>

void LightEngine::sunlightInit(Chunk *c)
{
	for (int x = 0; x < CHUNK_X; x++)
	{
		for (int z = 0; z < CHUNK_Z; z++)
		{
			c->setSunLight(x,CHUNK_Y-1,z,5);
			sunlightBfsQueue.emplace(x, CHUNK_Y-1, z, c);
		}
	}
	this->sunlightQueueClear();
}

void LightEngine::sunlightQueueClear()
{
	Chunk *chunk = NULL;
	// Currently not handling chunk edges
	while(sunlightBfsQueue.empty() == false)
	{
		// Get a reference to the front node. 
		LightNode &node = sunlightBfsQueue.front();
		chunk = node.chunk;
		// Pop the front node off the queue. We no longer need the node reference
		sunlightBfsQueue.pop();
		// Grab the light level of the current node
		short lightLevel = chunk->getSunLight(node.x, node.y, node.z);
		// cout << "1: "<< node.x << " " << node.y << " " << node.z << " light:" << lightLevel << endl;

		// x-1
		if (node.x - 1 >= 0)
		{
			if (chunk->getSunLight(node.x - 1, node.y, node.z) + 2 <= lightLevel)
			{
				// Set its light level
				chunk->setSunLight(node.x - 1, node.y, node.z, lightLevel - 1);
				// Emplace new node to queue. (could use push as well)
				if (!chunk->getBlock(node.x - 1, node.y, node.z)->isActive())
					sunlightBfsQueue.emplace(node.x - 1, node.y, node.z, chunk);
			}
		}
		// y-1
		if (node.y - 1 >= 0)
		{
			if (chunk->getSunLight(node.x, node.y - 1, node.z) + 2 <= lightLevel)
			{
				if (lightLevel)
					chunk->setSunLight(node.x, node.y - 1, node.z, lightLevel);
				// cout << "2: "<< node.x << " " << node.y << " " << node.z << " light:" << lightLevel << endl;
				// cout << "Next block active: " << chunk->getBlock(node.x, node.y - 1, node.z)->isActive() << endl;
				if (chunk->getBlock(node.x, node.y - 1, node.z)->isActive() == false)
					sunlightBfsQueue.emplace(node.x, node.y - 1, node.z, chunk);
			}
		}
		// z-1
		if (node.z - 1 >= 0)
		{
			if (chunk->getSunLight(node.x, node.y, node.z - 1) + 2 <= lightLevel)
			{
				chunk->setSunLight(node.x, node.y, node.z - 1, lightLevel - 1);
				if (!chunk->getBlock(node.x, node.y, node.z - 1)->isActive())
					sunlightBfsQueue.emplace(node.x, node.y, node.z - 1, chunk);
			}
		}
		// x+1
		if (node.x + 1 < CHUNK_X)
		{
			if (chunk->getSunLight(node.x + 1, node.y, node.z) + 2 <= lightLevel)
			{
				chunk->setSunLight(node.x + 1, node.y, node.z, lightLevel - 1);
				if (!chunk->getBlock(node.x + 1, node.y, node.z)->isActive())
					sunlightBfsQueue.emplace(node.x + 1, node.y, node.z, chunk);
			}
		}
		// y+1
		if (node.y + 1 < CHUNK_Y)
		{
			if (chunk->getSunLight(node.x, node.y + 1, node.z) + 2 <= lightLevel)
			{
				chunk->setSunLight(node.x, node.y + 1, node.z, lightLevel - 1);
				if (!chunk->getBlock(node.x, node.y + 1, node.z)->isActive())
					sunlightBfsQueue.emplace(node.x, node.y + 1, node.z, chunk);
			}
		}
		// z+1
		if (node.z + 1 < CHUNK_Z)
		{
			if (chunk->getSunLight(node.x, node.y, node.z + 1) + 2 <= lightLevel)
			{
				chunk->setSunLight(node.x, node.y, node.z + 1, lightLevel - 1);
				if (!chunk->getBlock(node.x, node.y, node.z + 1)->isActive())
					sunlightBfsQueue.emplace(node.x, node.y, node.z + 1, chunk);
			}
		}
	}
}

void LightEngine::lampLighting()
{
	// could make a chunk update list of glm::ivec3(chunk offsets) and return that to player.cpp to update chunks
	Chunk *chunk = NULL;
	
	// Currently not handling chunk edges
	while(lightBfsQueue.empty() == false)
	{
		// Get a reference to the front node. 
		LightNode &node = lightBfsQueue.front();
		chunk = node.chunk;
		// Pop the front node off the queue. We no longer need the node reference
		lightBfsQueue.pop();
		// Grab the light level of the current node
		short lightLevel = chunk->getTorchLight(node.x, node.y, node.z);

		// x-1
		if (node.x - 1 >= 0)
		{
			if (chunk->getTorchLight(node.x - 1, node.y, node.z) + 2 <= lightLevel)
			{
				// Set its light level
				chunk->setTorchLight(node.x - 1, node.y, node.z, lightLevel - 1);
				// Emplace new node to queue. (could use push as well)
				if (!chunk->getBlock(node.x - 1, node.y, node.z)->isActive())
					lightBfsQueue.emplace(node.x - 1, node.y, node.z, chunk);
			}
		}
		else if (chunk->getXMinus()) // add to neighbor
		{
			if (chunk->getXMinus()->getTorchLight(CHUNK_X - 1, node.y, node.z) + 2 <= lightLevel)
			{
				// if i can switch to just a light update that'd be best or a forced update all in one frame, OR render till can update
				chunk->getXMinus()->setState(UPDATE);
				chunk->getXMinus()->setTorchLight(CHUNK_X - 1, node.y, node.z, lightLevel - 1);
				if (!chunk->getXMinus()->getBlock(CHUNK_X - 1, node.y, node.z)->isActive())
					lightBfsQueue.emplace(CHUNK_X - 1, node.y, node.z, chunk->getXMinus());
			}
		}
		// y-1
		if (node.y - 1 >= 0)
		{
			if (chunk->getTorchLight(node.x, node.y - 1, node.z) + 2 <= lightLevel)
			{
				chunk->setTorchLight(node.x, node.y - 1, node.z, lightLevel - 1);
				if (!chunk->getBlock(node.x, node.y - 1, node.z)->isActive())
					lightBfsQueue.emplace(node.x, node.y - 1, node.z, chunk);
			}
		}
		// z-1
		if (node.z - 1 >= 0)
		{
			if (chunk->getTorchLight(node.x, node.y, node.z - 1) + 2 <= lightLevel)
			{
				chunk->setTorchLight(node.x, node.y, node.z - 1, lightLevel - 1);
				if (!chunk->getBlock(node.x, node.y, node.z - 1)->isActive())
					lightBfsQueue.emplace(node.x, node.y, node.z - 1, chunk);
			}
		}
		else if (chunk->getZMinus()) // add to neighbor
		{
			if (chunk->getZMinus()->getTorchLight(node.x, node.y, CHUNK_Z-1) + 2 <= lightLevel)
			{
				chunk->getZMinus()->setState(UPDATE);
				chunk->getZMinus()->setTorchLight(node.x, node.y, CHUNK_Z-1, lightLevel - 1);
				if (!chunk->getZMinus()->getBlock(node.x, node.y, CHUNK_Z-1)->isActive())
					lightBfsQueue.emplace(node.x, node.y, CHUNK_Z-1, chunk->getZMinus());
			}
		}
		// x+1
		if (node.x + 1 < CHUNK_X)
		{
			if (chunk->getTorchLight(node.x + 1, node.y, node.z) + 2 <= lightLevel)
			{
				chunk->setTorchLight(node.x + 1, node.y, node.z, lightLevel - 1);
				if (!chunk->getBlock(node.x + 1, node.y, node.z)->isActive())
					lightBfsQueue.emplace(node.x + 1, node.y, node.z, chunk);
			}
		}
		else if (chunk->getXPlus()) // add to neighbor
		{
			if (chunk->getXPlus()->getTorchLight(0, node.y, node.z) + 2 <= lightLevel)
			{
				chunk->getXPlus()->setState(UPDATE);
				chunk->getXPlus()->setTorchLight(0, node.y, node.z, lightLevel - 1);
				if (!chunk->getXPlus()->getBlock(0, node.y, node.z)->isActive())
					lightBfsQueue.emplace(0, node.y, node.z, chunk->getXPlus());
			}
		}
		// y+1
		if (node.y + 1 < CHUNK_Y)
		{
			if (chunk->getTorchLight(node.x, node.y + 1, node.z) + 2 <= lightLevel)
			{
				chunk->setTorchLight(node.x, node.y + 1, node.z, lightLevel - 1);
				if (!chunk->getBlock(node.x, node.y + 1, node.z)->isActive())
					lightBfsQueue.emplace(node.x, node.y + 1, node.z, chunk);
			}
		}
		// z+1
		if (node.z + 1 < CHUNK_Z)
		{
			if (chunk->getTorchLight(node.x, node.y, node.z + 1) + 2 <= lightLevel)
			{
				chunk->getZPlus()->setState(UPDATE);
				chunk->setTorchLight(node.x, node.y, node.z + 1, lightLevel - 1);
				if (!chunk->getBlock(node.x, node.y, node.z + 1)->isActive())
					lightBfsQueue.emplace(node.x, node.y, node.z + 1, chunk);
			}
		}
		else if (chunk->getZPlus()) // add to neighbor
		{
			if (chunk->getZPlus()->getTorchLight(node.x, node.y, 0) + 2 <= lightLevel)
			{
				chunk->getZPlus()->setTorchLight(node.x, node.y, 0, lightLevel - 1);
				if (!chunk->getZPlus()->getBlock(node.x, node.y, 0)->isActive())
					lightBfsQueue.emplace(node.x, node.y, 0, chunk->getZPlus());
			}
		}
	}
}


// SEGFAULTS WHEN I PLACE A LIGHT BLOCK IN THE AIR (ie. on a tree) THEN BREAK IT
void LightEngine::removedLighting()
{
	while(lightRemovalBfsQueue.empty() == false)
	{
		// Get a reference to the front node
		LightRemovalNode &node = lightRemovalBfsQueue.front();
		int lightLevel = (int)node.val;
		Chunk *chunk = node.chunk;
		// Pop the front node off the queue.
		lightRemovalBfsQueue.pop();

		// x-1
		if (node.x - 1 >= 0)
		{
			int neighborLevel = node.chunk->getTorchLight(node.x - 1, node.y, node.z);
			if (neighborLevel != 0 && neighborLevel < lightLevel)
			{
				// Set its light level
				node.chunk->setTorchLight(node.x - 1, node.y, node.z, 0);
				// Emplace new node to queue. (could use push as well)
				lightRemovalBfsQueue.emplace(node.x - 1, node.y, node.z, neighborLevel, chunk);
			}
			else if (neighborLevel >= lightLevel)
			{
				// Add it to the update queue, so it can propagate to fill in the gaps
				// left behind by this removal. We should update the lightBfsQueue after
				// the lightRemovalBfsQueue is empty.
				lightBfsQueue.emplace(node.x - 1, node.y, node.z, chunk);
			}
		}
		else if (chunk->getXMinus()) // add to neighbor
		{
			int neighborLevel = chunk->getXMinus()->getTorchLight(CHUNK_X - 1, node.y, node.z);
			if (neighborLevel != 0 && neighborLevel < lightLevel)
			{
				node.chunk->getXMinus()->setTorchLight(CHUNK_X-1, node.y, node.z, 0);
				lightRemovalBfsQueue.emplace(CHUNK_X-1, node.y, node.z, neighborLevel, chunk->getXMinus());
			}
			else if (neighborLevel >= lightLevel)
			{
				lightBfsQueue.emplace(CHUNK_X-1, node.y, node.z, chunk->getXMinus());
			}
		}

		// need to do neighbor light unloading
		// y-1
		if (node.y - 1 >= 0)
		{
			int neighborLevel = node.chunk->getTorchLight(node.x, node.y - 1, node.z);
			if (neighborLevel != 0 && neighborLevel < lightLevel)
			{
				node.chunk->setTorchLight(node.x, node.y - 1, node.z, 0);
				lightRemovalBfsQueue.emplace(node.x, node.y - 1, node.z, neighborLevel, chunk);
			}
			else if (neighborLevel >= lightLevel)
				lightBfsQueue.emplace(node.x, node.y - 1, node.z, chunk);
		}
		// z-1
		if (node.z - 1 >= 0)
		{
			int neighborLevel = node.chunk->getTorchLight(node.x, node.y, node.z - 1);
			if (neighborLevel != 0 && neighborLevel < lightLevel)
			{
				node.chunk->setTorchLight(node.x, node.y, node.z - 1, 0);
				lightRemovalBfsQueue.emplace(node.x, node.y, node.z - 1, neighborLevel, chunk);
			}
			else if (neighborLevel >= lightLevel)
				lightBfsQueue.emplace(node.x, node.y, node.z - 1, chunk);
		}
		else if (chunk->getZMinus()) // add to neighbor
		{
			int neighborLevel = chunk->getZMinus()->getTorchLight(node.x, node.y, CHUNK_Z-1);
			if (neighborLevel != 0 && neighborLevel < lightLevel)
			{
				node.chunk->getZMinus()->setTorchLight(node.x, node.y, CHUNK_Z-1, 0);
				lightRemovalBfsQueue.emplace(node.x, node.y, CHUNK_Z-1, neighborLevel, chunk->getZMinus());
			}
			else if (neighborLevel >= lightLevel)
			{
				lightBfsQueue.emplace(node.x, node.y, CHUNK_Z-1, chunk->getZMinus());
			}
		}

		// x+1
		if (node.x + 1 < CHUNK_X)
		{
			int neighborLevel = node.chunk->getTorchLight(node.x + 1, node.y, node.z);
			if (neighborLevel != 0 && neighborLevel < lightLevel)
			{
				node.chunk->setTorchLight(node.x + 1, node.y, node.z, 0);
				lightRemovalBfsQueue.emplace(node.x + 1, node.y, node.z, neighborLevel, chunk);
			}
			else if (neighborLevel >= lightLevel)
				lightBfsQueue.emplace(node.x + 1, node.y, node.z, chunk);
		}
		else if (chunk->getXPlus()) // add to neighbor
		{
			int neighborLevel = chunk->getXPlus()->getTorchLight(0, node.y, node.z);
			if (neighborLevel != 0 && neighborLevel < lightLevel)
			{
				node.chunk->getXPlus()->setTorchLight(0, node.y, node.z, 0);
				lightRemovalBfsQueue.emplace(0, node.y, node.z, neighborLevel, chunk->getXPlus());
			}
			else if (neighborLevel >= lightLevel)
			{
				lightBfsQueue.emplace(0, node.y, node.z, chunk->getXPlus());
			}
		}

		// y-1
		if (node.y + 1 < CHUNK_Y)
		{
			int neighborLevel = node.chunk->getTorchLight(node.x, node.y + 1, node.z);
			if (neighborLevel != 0 && neighborLevel < lightLevel)
			{
				node.chunk->setTorchLight(node.x, node.y + 1, node.z, 0);
				lightRemovalBfsQueue.emplace(node.x, node.y + 1, node.z, neighborLevel, chunk);
			}
			else if (neighborLevel >= lightLevel)
				lightBfsQueue.emplace(node.x, node.y + 1, node.z, chunk);
		}

		// z-1
		if (node.z + 1 < CHUNK_Z)
		{
			int neighborLevel = node.chunk->getTorchLight(node.x, node.y, node.z + 1);
			if (neighborLevel != 0 && neighborLevel < lightLevel)
			{
				node.chunk->setTorchLight(node.x, node.y, node.z + 1, 0);
				lightRemovalBfsQueue.emplace(node.x, node.y, node.z + 1, neighborLevel, chunk);
			}
			else if (neighborLevel >= lightLevel)
				lightBfsQueue.emplace(node.x, node.y, node.z + 1, chunk);
		}
		else if (chunk->getZPlus()) // add to neighbor
		{
			int neighborLevel = chunk->getZPlus()->getTorchLight(node.x, node.y, 0);
			if (neighborLevel != 0 && neighborLevel < lightLevel)
			{
				node.chunk->getZPlus()->setTorchLight(node.x, node.y, 0, 0);
				lightRemovalBfsQueue.emplace(node.x, node.y, 0, neighborLevel, chunk->getZPlus());
			}
			else if (neighborLevel >= lightLevel)
			{
				lightBfsQueue.emplace(node.x, node.y, 0, chunk->getZPlus());
			}
		}
	}
	this->lampLighting();
}

