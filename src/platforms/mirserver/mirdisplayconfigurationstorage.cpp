#include "mirdisplayconfigurationstorage.h"

namespace qtmir {
    
MirDisplayConfigurationStorage::MirDisplayConfigurationStorage()
{
}

MirDisplayConfigurationStorage::~MirDisplayConfigurationStorage()
{
}

bool MirDisplayConfigurationStorage::load(const miroil::DisplayId&, miroil::DisplayConfigurationOptions&) const
{
    return false;    
}

void MirDisplayConfigurationStorage::save(const miroil::DisplayId&, const miroil::DisplayConfigurationOptions&)
{
}

}

