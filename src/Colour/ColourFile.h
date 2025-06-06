//
// Created by DexrnZacAttack on 1/2/2025.
//

#ifndef COLOURFILE_H
#define COLOURFILE_H
#include "ColourFileCommons.h"
#include "Colours/WorldColour.h"

namespace lce::colour {

class LIBLCE_API ColourFile : public ColourFileCommons {
    public:
    std::vector<WorldColour> worldColours;

    ColourFile();
    ColourFile(std::vector<Colour> colours, std::vector<WorldColour> worldColours);

    static ColourFile read(std::vector<uint8_t> data);
    uint64_t getSize() const;

    uint8_t *create() const;
};

} // lce::colour

#endif //COLOURFILE_H
