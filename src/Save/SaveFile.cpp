//
// Created by DexrnZacAttack on 12/19/2024.
//

#include "SaveFile.h"

#include <chrono>
#include <vector>

#include "IndexInnerFile.h"
#include "../IO/BinaryIO.h"

namespace lce::save {
    SaveFile::SaveFile(uint32_t indexOffset, uint32_t indexFileCount, uint16_t origVersion, uint16_t version,
        const std::vector<std::shared_ptr<IndexInnerFile>> &index) {
    }

    SaveFile::SaveFile() = default;

    SaveFile::SaveFile(ByteOrder endian) {
        this->endian = endian;
    }

    /**
     * Reads a save file from a pointer to the data
     * @param data The data you want to read (a save file)
     * @return The save file.
     */
    SaveFile::SaveFile(std::vector<uint8_t> data, ByteOrder endian) {
        this->endian = endian;
        io::BinaryIO io(data.data());

        this->indexOffset = io.read<uint32_t>(this->endian);

        if (this->indexOffset > data.size())
            throw std::runtime_error("Index offset points to an area that is out of bounds of the data given.");
		
		DebugLog(""<<io.getPosition());
        this->setIndexCount(io.read<uint32_t>(this->endian));

        if (this->getIndexCount() > (0xFFFFFFFF - HEADER_SIZE) / this->SaveFile::getIndexEntrySize() )
            throw std::runtime_error("File count (" + std::to_string(this->getIndexCount() ) + ") makes the file too big for it's index offset to stored in a 32-bit integer.");

        this->originalVersion = io.read<uint16_t>(this->endian);
        this->version = io.read<uint16_t>(this->endian);

        DebugLog("Index offset: " << this->indexOffset);
        DebugLog("Index file count: " << getIndexCount());
        DebugLog("Version: " << this->version);
        DebugLog("Orig version: " << this->originalVersion);

        for (int i = 0; i < this->getIndexCount(); ++i) {
            io.seek(this->indexOffset + (144 * i));
            // read the index entry
            IndexInnerFile inf = IndexInnerFile(io.readOfSize(144), false, this->endian);
            
            DebugLog(inf.getName());

            // read the data, maybe should be changed
            io.seek(inf.getOffset());
            inf.setData(new uint8_t[inf.getSize()]);
            io.readInto(inf.getData(), inf.getSize());
            
            // set the entry
            addFile(std::make_shared<IndexInnerFile>(inf));
        }
        
        for (auto& file : getIndex() ) {
			std::shared_ptr<IndexInnerFile> innerFile = std::dynamic_pointer_cast<IndexInnerFile>(file);
			
			innerFile->setOffset(io.getPosition());
            innerFile->setTimestamp(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
            io.writeBytes(innerFile->getData(), innerFile->getSize());
        }
    }

    /**
     * Writes the save file
     * @return Pointer to the save file
     */
    uint8_t* SaveFile::create() const {
        io::BinaryIO io(this->getSize());

        uint32_t indexOffset = calculateIndexOffset();

        if (this->getIndexOffset() > 0xFFFFFFFF - HEADER_SIZE)
            throw std::runtime_error("Index offset is too big to be stored in a 32-bit integer.");

        io.write<uint32_t>(0, this->endian);
        io.write<uint32_t>(getIndexCount(), this->endian);
        io.write<uint16_t>(this->originalVersion, this->endian);
        io.write<uint16_t>(this->version, this->endian);

        io.seek(this->indexOffset);
        
        for (auto& file: getIndex() ) {
			const std::shared_ptr<const IndexInnerFile> innerFile = std::dynamic_pointer_cast<const IndexInnerFile>(file);
			
            DebugLog(innerFile->getName().length());
            
            io.writeWChar2Byte(innerFile->getNameU16(), this->endian, false);
            io.write<uint32_t>(innerFile->getSize(), this->endian);
            io.write<uint32_t>(innerFile->getOffset(), this->endian);
            io.write<uint64_t>(innerFile->getTimestamp(), this->endian);
        }

        io.seek(0);
        io.write<uint32_t>(this->indexOffset, this->endian);

        return io.getData();
    }

    /**
     * Gets the size of an index entry based on the save file class type.
     * @return The size of an index entry
     */
    uint32_t SaveFile::getIndexEntrySize() const {
        return 144;
    }
} // lce
