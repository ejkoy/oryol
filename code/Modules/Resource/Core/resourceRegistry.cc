//------------------------------------------------------------------------------
//  resourceRegistry.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "resourceRegistry.h"

namespace Oryol {
namespace _priv {

//------------------------------------------------------------------------------
resourceRegistry::resourceRegistry() :
isValid(false) {
    // empty
}

//------------------------------------------------------------------------------
resourceRegistry::~resourceRegistry() {
    o_assert_dbg(!this->isValid);
    o_assert_dbg(this->entries.Empty());
}

//------------------------------------------------------------------------------
void
resourceRegistry::Setup(int32 reserveSize) {
    o_assert_dbg(!this->isValid);
    
    this->isValid = true;
    this->entries.Reserve(reserveSize);
    this->locatorIndexMap.Reserve(reserveSize);
    this->idIndexMap.Reserve(reserveSize);
}

//------------------------------------------------------------------------------
void
resourceRegistry::Discard() {
    o_assert_dbg(this->isValid);
    
    this->entries.Clear();
    this->locatorIndexMap.Clear();
    this->idIndexMap.Clear();
    this->isValid = false;
}

//------------------------------------------------------------------------------
bool
resourceRegistry::IsValid() const {
    return this->isValid;
}

//------------------------------------------------------------------------------
void
resourceRegistry::Add(const Locator& loc, Id id, ResourceLabel label) {
    o_assert_dbg(this->isValid);
    o_assert_dbg(id.IsValid());
    o_assert(!this->idIndexMap.Contains(id));
    
    this->entries.Add(loc, id, label);
    if (loc.IsShared()) {
        o_assert_dbg(!this->locatorIndexMap.Contains(loc));
        this->locatorIndexMap.Add(loc, this->entries.Size() - 1);
    }
    this->idIndexMap.Add(id, this->entries.Size() - 1);
}

//------------------------------------------------------------------------------
const resourceRegistry::Entry*
resourceRegistry::findEntryByLocator(const Locator& loc) const {
    if (loc.IsShared()) {
        const int32 mapIndex = this->locatorIndexMap.FindIndex(loc);
        if (InvalidIndex != mapIndex) {
            const int32 entryIndex = this->locatorIndexMap.ValueAtIndex(mapIndex);
            return &(this->entries[entryIndex]);
        }
    }
    return nullptr;
}

//------------------------------------------------------------------------------
const resourceRegistry::Entry*
resourceRegistry::findEntryById(Id id) const {
    const int32 mapIndex = this->idIndexMap.FindIndex(id);
    if (InvalidIndex != mapIndex) {
        const int32 entryIndex = this->idIndexMap.ValueAtIndex(mapIndex);
        return &(this->entries[entryIndex]);
    }
    return nullptr;
}

//------------------------------------------------------------------------------
bool
resourceRegistry::Contains(Id id) const {
    o_assert_dbg(this->isValid);
    o_assert_dbg(id.IsValid());
    return this->idIndexMap.Contains(id);
}

//------------------------------------------------------------------------------
Id
resourceRegistry::Lookup(const Locator& loc) const {
    o_assert_dbg(this->isValid);
    if (loc.IsShared()) {
        const Entry* entry = this->findEntryByLocator(loc);
        if (nullptr != entry) {
            return entry->id;
        }
    }
    return Id::InvalidId();
}

//------------------------------------------------------------------------------
Array<Id>
resourceRegistry::Remove(ResourceLabel label) {
    o_assert_dbg(this->isValid);
    Array<Id> removed;
    removed.Reserve(this->entries.Size() < 256 ? this->entries.Size() : 256);
    
    // for each entry where id.label matches label (from behind
    // because matching entries will be removed)
    // FIXME: this can be slow if many resource are live!
    int32 entryIndex = this->entries.Size() - 1;
    for (; entryIndex >= 0; entryIndex--) {
        if ((ResourceLabel::All == label) || (this->entries[entryIndex].label == label)) {
            Id id = this->entries[entryIndex].id;
            Locator loc = this->entries[entryIndex].locator;
            removed.Add(id);
            
            // remove entries
            this->entries.EraseSwapBack(entryIndex);
            this->idIndexMap.Erase(id);
            if (loc.IsShared()) {
                this->locatorIndexMap.Erase(loc);
            }
            
            // fixup the index maps (see elementBuffer)
            const int32 swappedIndex = this->entries.Size();
            if (entryIndex != swappedIndex) {
                for (auto& elm : this->idIndexMap) {
                    if (swappedIndex == elm.value) {
                        elm.value = entryIndex;
                        break;
                    }
                }
                for (auto& elm : this->locatorIndexMap) {
                    if (swappedIndex == elm.value) {
                        elm.value = entryIndex;
                        break;
                    }
                }
            }
            
            // make sure nothing broke
            #if ORYOL_DEBUG
            o_assert(this->checkIntegrity());
            #endif
        }
    }
    return removed;
}

//------------------------------------------------------------------------------
const Locator&
resourceRegistry::GetLocator(Id id) const {
    o_assert_dbg(this->isValid);
    o_assert_dbg(id.IsValid());
    
    const Entry* entry = this->findEntryById(id);
    o_assert_dbg(nullptr != entry);
    return entry->locator;
}

//------------------------------------------------------------------------------
ResourceLabel
resourceRegistry::GetLabel(Id id) const {
    o_assert_dbg(this->isValid);
    o_assert_dbg(id.IsValid());
    
    const Entry* entry = this->findEntryById(id);
    o_assert_dbg(nullptr != entry);
    return entry->label;
}

//------------------------------------------------------------------------------
int32
resourceRegistry::GetNumResources() const {
    o_assert_dbg(this->isValid);
    return this->entries.Size();
}

//------------------------------------------------------------------------------
Id
resourceRegistry::GetIdByIndex(int32 index) const {
    o_assert_dbg(this->isValid);
    return this->entries[index].id;
}

//------------------------------------------------------------------------------
#if ORYOL_DEBUG
bool
resourceRegistry::checkIntegrity() const {
    for (const auto& kvp : this->locatorIndexMap) {
        const Locator& loc = kvp.key;
        const int32 entryIndex = kvp.value;
        const Locator& entryLoc = this->entries[entryIndex].locator;
        if (entryLoc != loc) {
            o_error("ResourceRegistry: locator mismatch at index '%d' (%s != %s)\n",
                    entryIndex, entryLoc.Location().AsCStr(), loc.Location().AsCStr());
            return false;
        }
    }
    for (const auto& kvp : this->idIndexMap) {
        const Id& id = kvp.key;
        const int32 entryIndex = kvp.value;
        const Id& entryId = this->entries[entryIndex].id;
        if (entryId != id) {
            o_error("ResourceRegistry:: id mismatch at index '%d' (%d,%d,%d != %d,%d,%d)\n",
                    entryIndex, entryId.UniqueStamp, entryId.SlotIndex, entryId.Type,
                    id.UniqueStamp, id.SlotIndex, id.Type);
            return false;
        }
    }
    return true;
}
#endif

} // namespace _priv
} // namespace Oryol
