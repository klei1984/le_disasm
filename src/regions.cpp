/* Copyright (C) 2010  Unavowed <unavowed@vexillium.org>
 * Copyright (C) 2017  samunders-core <samunders-core@users.noreply.github.com>
 * Copyright (C) 2019-2025  klei1984 <53688147+klei1984@users.noreply.github.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "regions.hpp"

#include <cassert>
#include <iostream>

#include "image_object.hpp"
#include "print.hpp"

Regions::Regions(std::vector<ImageObject>& objects, bool verbose) {
    this->verbose = verbose;
    for (size_t n = 0; n < objects.size(); ++n) {
        ImageObject& obj = objects[n];
        Type type = obj.IsExecutable() ? UNKNOWN : DATA;
        PrintAddress(std::cerr, obj.BaseAddress(), "Creating Region (0x")
            << ", " << std::dec << obj.Size() << ", " << type << ")" << std::endl;
        regions[obj.BaseAddress()] = Region(obj.BaseAddress(), obj.Size(), type, std::addressof(obj));
        if (type == DATA) {
            label_types[obj.BaseAddress()] = type;
        }
    }
}

uint32_t Regions::GetLabelType(uint32_t address, Type* label) {
    const std::map<uint32_t, Type>::iterator item = label_types.find(address);
    if (label_types.end() != item) {
        *label = item->second;
        return item->first;
    }
    return 0;
}

Region* Regions::RegionContaining(uint32_t address) {
    std::map<uint32_t, Region>::iterator itr = regions.lower_bound(address);
    if (regions.end() != itr) {
        if (itr->first == address) {
            return &itr->second;
        } else if (regions.begin() == itr) {
            return NULL;
        }
    }
    if (regions.empty()) {
        return NULL;
    }
    --itr;
    return itr->second.ContainsAddress(address) ? &itr->second : NULL;
}

void Regions::SplitInsert(Region& parent, const Region& target) {
    assert(parent.ContainsAddress(target.Address()));
    assert(parent.ContainsAddress(target.EndAddress() - 1));

    Region reg = target;
    reg.ImageObjectPointer(parent.ImageObjectPointer());
    FlagsRestorer _(std::cerr);
    Region next(reg.EndAddress(), parent.EndAddress() - reg.EndAddress(), parent.GetType(), parent.ImageObjectPointer());
    if (verbose) std::cerr << parent << " split to ";

    if (reg.Address() != parent.Address()) {
        parent.Size(reg.Address() - parent.Address());
        regions[reg.Address()] = reg;
        if (verbose) std::cerr << parent << ", " << reg;
    } else {
        parent = reg;
        if (verbose) std::cerr << parent;
    }

    if (next.Size() > 0) {
        regions[reg.EndAddress()] = next;
        if (verbose) std::cerr << ", " << next;
    }
    if (verbose) std::cerr << std::endl;

    assert(reg.ImageObjectPointer());
    assert(next.ImageObjectPointer());
    assert(parent.ImageObjectPointer());
    CheckMergeRegions(reg.Address());
}

Region* Regions::NextRegion(const Region& reg) {
    std::map<uint32_t, Region>::iterator itr = regions.upper_bound(reg.Address());
    return regions.end() != itr ? &itr->second : NULL;
}

Region* Regions::PreviousRegion(const Region& reg) {
    for (std::map<uint32_t, Region>::iterator itr = regions.lower_bound(reg.Address()); regions.begin() != itr;) {
        --itr;
        return &itr->second;
    }
    return NULL;
}

void Regions::CheckMergeRegions(uint32_t address) {
    Region& reg = regions[address];
    Region* merged = AttemptMerge(PreviousRegion(reg), &reg);
    AttemptMerge(merged, NextRegion(*merged));
}

Region* Regions::AttemptMerge(Region* prev, Region* next) {
    if (prev != NULL and next != NULL and prev->GetType() == next->GetType() and prev->EndAddress() == next->Address() and
        prev->GetBitness() == next->GetBitness()) {
        if (verbose) std::cerr << "Combining " << *prev << " and " << *next << std::endl;
        prev->Size(prev->Size() + next->Size());
        regions.erase(next->Address());
        return prev;
    }
    return next;
}
