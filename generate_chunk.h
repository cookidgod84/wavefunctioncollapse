
#include <vector>
#include <set>
#include <random>
#include <limits>

class Ctileset {
    //position in vector is id
    public:
        std::vector<float> tileweights;
        float total_weighting = 0;
        std::vector<std::set<int>> allowed_north_tiles;
        std::vector<std::set<int>> allowed_east_tiles;
        std::vector<std::set<int>> allowed_south_tiles;
        std::vector<std::set<int>> allowed_west_tiles;
};
class Cchunk {
    public:
        const static int CHUNK_SIZE = 16;
        int chunkids[CHUNK_SIZE][CHUNK_SIZE];
        float chunkentropies[CHUNK_SIZE][CHUNK_SIZE];
        std::set<int> allowed_tiles[CHUNK_SIZE][CHUNK_SIZE];
        void initChunk(Ctileset tileset) {
            std::set<int> temp;
            for(int i = 0;i<tileset.tileweights.size();i++) {
                temp.insert(i);
            }
            for(int x = 0;x<CHUNK_SIZE-1;x++) {
                for(int y = 0;y<CHUNK_SIZE-1;y++) {
                    chunkids[x][y] = -1;
                    chunkentropies[x][y] = 10000000;//10 million should be large enough. cannot use limits::max here as it collides with findLowestEntropy comparisions
                    allowed_tiles[x][y] = temp;
                }
            }
        }
};
