#pragma once
#include <miroil/display_configuration_storage.h>

namespace qtmir {

class MirDisplayConfigurationStorage : public miroil::DisplayConfigurationStorage
{
public:
    MirDisplayConfigurationStorage();
    ~MirDisplayConfigurationStorage();
    
    void save(const miroil::DisplayId&, const miroil::DisplayConfigurationOptions&) override;
    bool load(const miroil::DisplayId&, miroil::DisplayConfigurationOptions&) const override;
};

}


