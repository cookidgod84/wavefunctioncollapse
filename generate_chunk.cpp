#include <vector>
#include <set>
#include <random>
#include <limits>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <format>
#include "generate_chunk.h"
#include <chrono>

/**
 * test function to display generated chunk
 */
void displayChunk(Cchunk* chunk) {
    std::cout<<std::endl;
    for (int x = 0; x<chunk->CHUNK_SIZE-1; x++) {
		for (int y = 0; y<chunk->CHUNK_SIZE-1; y++) {
            std::cout<<chunk->chunkids[y][x]<<" ";
        }
        std::cout<<std::endl;
    }
}

/**
 * finds tile with the lowest entropy in the chunk. lowest pos should be an array for x and y coords
 */
void findLowestEntropy(Cchunk* chunk, int* lowestpos) {
    float lowestval = std::numeric_limits<float>::max();
    for (int x = 0; x < chunk->CHUNK_SIZE-1 ; x++) {
		for (int y = 0; y < chunk->CHUNK_SIZE-1; y++) {
            if (chunk->chunkentropies[x][y] > 0 && chunk->chunkentropies[x][y] < lowestval) {
				lowestval = chunk->chunkentropies[x][y];
				lowestpos[0] = x;
                lowestpos[1] = y;
			}
        }
    }
};

/**
 * finds intersection between two sets of tileids
 */
std::set<int> getTileIntersection(std::set<int> seta, std::set<int> setb)
{	
	std::set<int> returner;
	std::set_intersection(seta.begin(), seta.end(), setb.begin(), setb.end(), inserter(returner, returner.begin()));
	return returner;
};


/** 
 * calculates entropy of a given tile
 */
void calculateEntropy(std::set<int> allowed_tiles, Ctileset tileset, float* entropyptr) {
    *entropyptr = 0;
    std::set<int>::iterator it;
    for(it = allowed_tiles.begin();it!=allowed_tiles.end(); ++it) {
        int tileweight = tileset.tileweights[*it];
        *entropyptr = *entropyptr + tileweight * log(tileweight);
    }
}

/**
 * randomly decides what tile should be used for a given position in the chunk
 */
void fillTile(Cchunk* chunk, int* pos, std::mt19937 random, Ctileset tileset) {
    float tileweightsum = 0.0;
    std::set<int>::iterator it;
    for(it = chunk->allowed_tiles[pos[0]][pos[1]].begin();it!=chunk->allowed_tiles[pos[0]][pos[1]].end(); ++it) {
        tileweightsum+=tileset.tileweights[*it];
    }
    std::uniform_real_distribution<float> dist{0.0,tileweightsum};
    
    float randomnum = dist(random);
	float cdf = 0;
    for(it = chunk->allowed_tiles[pos[0]][pos[1]].begin();it!=chunk->allowed_tiles[pos[0]][pos[1]].end(); ++it) {
		cdf = cdf + tileset.tileweights[*it];
		if (cdf > randomnum) {
			chunk->chunkids[pos[0]][pos[1]] = *it;
            chunk->allowed_tiles[pos[0]][pos[1]] = {chunk->chunkids[pos[0]][pos[1]]};
            chunk->chunkentropies[pos[0]][pos[1]] = std::numeric_limits<float>::max();
			return;
		}
	}
}

void updateFilledTileNeighbours(Cchunk* chunk, Ctileset tileset, int* filledtilepos) {
    int northpos[2] = {filledtilepos[0],filledtilepos[1]-1};
    int eastpos[2] = {filledtilepos[0]+1,filledtilepos[1]};
    int southpos[2] = {filledtilepos[0],filledtilepos[1]+1};
    int westpos[2] = {filledtilepos[0]-1,filledtilepos[1]};
    //attempt two

    //north tiles
    if(filledtilepos[1] > 1) {
        for(std::set<int>::iterator it = chunk->allowed_tiles[northpos[0]][northpos[1]].begin();it!=chunk->allowed_tiles[northpos[0]][northpos[1]].end();) {
            std::set<int> intersection = getTileIntersection(tileset.allowed_south_tiles[*it],{chunk->chunkids[filledtilepos[0]][filledtilepos[1]]});
            if(intersection.size() == 0) {
                it = chunk->allowed_tiles[northpos[0]][northpos[1]].erase(it);
            } else {
                ++it;
            }
        }
    }
    //east tiles
    if(filledtilepos[0] < chunk->CHUNK_SIZE-2) {
        for(std::set<int>::iterator it = chunk->allowed_tiles[eastpos[0]][eastpos[1]].begin();it!=chunk->allowed_tiles[eastpos[0]][eastpos[1]].end();) {
            std::set<int> intersection = getTileIntersection(tileset.allowed_west_tiles[*it],{chunk->chunkids[filledtilepos[0]][filledtilepos[1]]});
            std::set<int>::iterator tempit;
            if(intersection.size() == 0) {
                it = chunk->allowed_tiles[eastpos[0]][eastpos[1]].erase(it);
            } else {
                ++it;
            }
        }
    }
    //south tiles
    if(filledtilepos[1] < chunk->CHUNK_SIZE-2) {
        for(std::set<int>::iterator it = chunk->allowed_tiles[southpos[0]][southpos[1]].begin();it!=chunk->allowed_tiles[southpos[0]][southpos[1]].end();) {
            std::set<int> intersection = getTileIntersection(tileset.allowed_north_tiles[*it],{chunk->chunkids[filledtilepos[0]][filledtilepos[1]]});
            if(intersection.size() == 0) {
                it = chunk->allowed_tiles[southpos[0]][southpos[1]].erase(it);
            } else {
                ++it;
            }
        }
    }
    //west tiles
    if(filledtilepos[0] > 1) {
        for(std::set<int>::iterator it = chunk->allowed_tiles[westpos[0]][westpos[1]].begin();it!=chunk->allowed_tiles[westpos[0]][westpos[1]].end();) {
            std::set<int> intersection = getTileIntersection(tileset.allowed_east_tiles[*it],{chunk->chunkids[filledtilepos[0]][filledtilepos[1]]});
            if(intersection.size() == 0) {
                it = chunk->allowed_tiles[westpos[0]][westpos[1]].erase(it);
            } else {
                ++it;
            }
        }
    }

}

void testFunc(Ctileset tileset, Cchunk* chunk) {
    int lowest_entropy_pos[2];
    lowest_entropy_pos[1] = 0;
    srand(time(NULL));
    std::random_device rd;
    std::mt19937 random(rd());
    std::cout<<std::endl;
    for(int i = 0;i<chunk->CHUNK_SIZE * chunk->CHUNK_SIZE-1;i++) {
        findLowestEntropy(chunk, lowest_entropy_pos);
        random();//needed for different random num in every fillTile() call
        fillTile(chunk,lowest_entropy_pos,random,tileset);
        updateFilledTileNeighbours(chunk,tileset,lowest_entropy_pos);
    }
    //displayChunk(chunk);
}

int main() {
    Ctileset temptileset;
    int tilesetchoice = 1;
    if(tilesetchoice == 0) {
        temptileset.tileweights = {2.0,2.0,2.0};
        temptileset.total_weighting = 6.0;
        temptileset.allowed_north_tiles = {{0,1},{0,1,2},{1,2}};
        temptileset.allowed_east_tiles = {{0,1},{0,1,2},{1,2}};
        temptileset.allowed_south_tiles = {{0,1},{0,1,2},{1,2}};
        temptileset.allowed_west_tiles = {{0,1},{0,1,2},{1,2}};
    } else if(tilesetchoice == 1) {
        temptileset.tileweights = {2.0,2.0,2.0,2.0,2.0};
        temptileset.total_weighting = 10.0;
        temptileset.allowed_north_tiles = {{0,3},{0,3},{1,2,4},{1,2,4},{1,2,4}};
        temptileset.allowed_east_tiles = {{0,4},{1,2,3},{0,4},{1,2,3},{1,2,3}};
        temptileset.allowed_south_tiles = {{0,1},{2,3,4},{2,3,4},{0,1},{2,3,4}};
        temptileset.allowed_west_tiles = {{0,2},{1,3,4},{1,3,4},{1,3,4},{0,2}};
    }

    std::set<int> set1 = {2};
    std::set<int> set2 = {1,2,3,4};
    std::set<int> set3 = {};
    std::set<int> set4 = {1,2,3};
    std::set<int> set5 = {1,4};
    //std::cout<<getTileIntersection(set1,set2)<<std::endl;
    Cchunk tempchunk;
    tempchunk.initChunk(temptileset);
    
    auto start = std::chrono::high_resolution_clock::now();
    testFunc(temptileset,&tempchunk);
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the duration
    std::chrono::duration<double> duration = end - start;

    std::cout << "Time taken: " << duration.count() << std::endl; 
    //collapseChunk(temptileset,&tempchunk);
    //displayChunk(tempchunk);
}