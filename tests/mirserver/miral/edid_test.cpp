/*
 * Copyright (C) 2014-2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <numeric>

#include "edid.h"

using namespace miral;

using TestDataParamType =
    std::tuple<std::vector<uint8_t>, std::string, std::string, uint16_t, uint32_t>;

std::vector<TestDataParamType> testData {
    TestDataParamType {
        {
            0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x09, 0xe5, 0xac, 0x06, 0x00, 0x00, 0x00, 0x00,
            0x03, 0x19, 0x01, 0x04, 0x95, 0x1f, 0x11, 0x78, 0x02, 0x9d, 0x40, 0x9a, 0x5d, 0x55, 0x8d, 0x28,
            0x1e, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xbc, 0x39, 0x80, 0x18, 0x71, 0x38, 0x28, 0x40, 0x30, 0x20,
            0x35, 0x00, 0x35, 0xad, 0x10, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x42,
            0x4f, 0x45, 0x20, 0x44, 0x54, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfe,
            0x00, 0x4e, 0x56, 0x31, 0x34, 0x30, 0x46, 0x48, 0x4d, 0x2d, 0x4e, 0x34, 0x35, 0x0a, 0x00, 0xb7
        }, // 128 byte edid data
        "BOE", // vendor
        "", // monitor name
        1708, // product code
        0 // serial number
    },
    TestDataParamType {
        {
            // 256 byte edid
            0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x4c, 0x2d, 0xd6, 0x08, 0x31, 0x37, 0x31, 0x30,
            0x12, 0x16, 0x01, 0x03, 0x80, 0x33, 0x1d, 0x78, 0x2a, 0x01, 0xf1, 0xa2, 0x57, 0x52, 0x9f, 0x27,
            0x0a, 0x50, 0x54, 0xbf, 0xef, 0x80, 0x71, 0x4f, 0x81, 0xc0, 0x81, 0x00, 0x81, 0x80, 0x95, 0x00,
            0xa9, 0xc0, 0xb3, 0x00, 0x01, 0x01, 0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
            0x45, 0x00, 0xfe, 0x1f, 0x11, 0x00, 0x00, 0x1e, 0x01, 0x1d, 0x00, 0x72, 0x51, 0xd0, 0x1e, 0x20,
            0x6e, 0x28, 0x55, 0x00, 0xfe, 0x1f, 0x11, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x32,
            0x4b, 0x1e, 0x51, 0x11, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc,
            0x00, 0x53, 0x32, 0x33, 0x42, 0x33, 0x35, 0x30, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x11,
            0x02, 0x03, 0x11, 0xb1, 0x46, 0x90, 0x04, 0x1f, 0x13, 0x12, 0x03, 0x65, 0x03, 0x0c, 0x00, 0x10,
            0x00, 0x01, 0x1d, 0x00, 0xbc, 0x52, 0xd0, 0x1e, 0x20, 0xb8, 0x28, 0x55, 0x40, 0xfe, 0x1f, 0x11,
            0x00, 0x00, 0x1e, 0x8c, 0x0a, 0xd0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0c, 0x40, 0x55, 0x00, 0xfe,
            0x1f, 0x11, 0x00, 0x00, 0x18, 0x8c, 0x0a, 0xd0, 0x8a, 0x20, 0xe0, 0x2d, 0x10, 0x10, 0x3e, 0x96,
            0x00, 0xfe, 0x1f, 0x11, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb4
        }, // 256 byte edid data
        "SAM", // vendor
        "S23B350", // monitor name
        2262, // product code
        808531761 // serial number
    }
};

class EdidTest :
    public ::testing::TestWithParam<TestDataParamType> {};

TEST(EdidTest, Construct)
{
    Edid edid;
    EXPECT_EQ(edid.vendor, "");
    EXPECT_EQ(edid.product_code, 0);
    EXPECT_EQ(edid.serial_number, uint32_t(0));
    EXPECT_EQ(edid.size.width, 0); EXPECT_EQ(edid.size.height, 0);

    char zero_array[13] = {0};
    for (int i = 0; i < 4; i++) {
        EXPECT_EQ(edid.descriptors[i].type, Edid::Descriptor::Type::undefined);

        auto& value = edid.descriptors[i].value;
        EXPECT_TRUE(std::equal(std::begin(value.monitor_name), std::end(value.monitor_name), std::begin(zero_array)));
        EXPECT_TRUE(std::equal(std::begin(value.unspecified_text), std::end(value.unspecified_text), std::begin(zero_array)));
        EXPECT_TRUE(std::equal(std::begin(value.serial_number), std::end(value.serial_number), std::begin(zero_array)));
    }
}

TEST_P(EdidTest, Test_InvalidChecksum)
{
    const std::vector<uint8_t>& data = std::get<0>(GetParam());

    std::vector<uint8_t> invalidChecksum{data};
    invalidChecksum[8] = invalidChecksum[8]+1;

    Edid edid;
    try {
        edid.parse_data(invalidChecksum);
    } catch(std::runtime_error const& err) {
        EXPECT_EQ(err.what(), std::string("Invalid EDID checksum"));
        return;
    }
    FAIL() << "Expected std::runtime_error(\"Invalid EDID checksum\")";
}

TEST_P(EdidTest, Test_InvalidHeader)
{
    const std::vector<uint8_t>& data = std::get<0>(GetParam());

    std::vector<uint8_t> invalidHeader{data};
    invalidHeader[1] = 0xfe;

    uint8_t checksum = std::accumulate(invalidHeader.begin(), invalidHeader.end()-1, 0);
    invalidHeader[invalidHeader.size()-1] = (uint8_t)(~checksum+1);

    Edid edid;
    try {
        edid.parse_data(invalidHeader);
    } catch(std::runtime_error const& err) {
        EXPECT_EQ(err.what(), std::string("Invalid EDID header"));
        return;
    }
    FAIL() << "Expected std::runtime_error(\"Invalid EDID header\")";
}

TEST_P(EdidTest, Test_Valid)
{
    const std::vector<uint8_t>& data = std::get<0>(GetParam());

    Edid edid;
    EXPECT_NO_THROW(edid.parse_data(data));
}

TEST_P(EdidTest, CheckData)
{
    const std::vector<uint8_t>& data = std::get<0>(GetParam());
    const std::string& vendor = std::get<1>(GetParam());
    const std::string& monitor_name = std::get<2>(GetParam());
    const uint16_t product_code = std::get<3>(GetParam());
    const uint32_t serial_number = std::get<4>(GetParam());

    Edid edid;
    EXPECT_NO_THROW(edid.parse_data(data));

    ASSERT_EQ(edid.vendor, vendor);
    ASSERT_EQ(edid.product_code, product_code);
    ASSERT_EQ(edid.serial_number, serial_number);

    if (monitor_name != "") {
        bool found_monitor_name = false;
        for (int i = 0; i < 4; i++) {
            if (edid.descriptors[i].type == Edid::Descriptor::Type::monitor_name ) {
                found_monitor_name = true;
                ASSERT_EQ(std::string(&edid.descriptors[i].value.monitor_name[0]), monitor_name);
            }
        }
        ASSERT_TRUE(found_monitor_name);
    }
}

INSTANTIATE_TEST_CASE_P(AllEdidTests,
                        EdidTest,
                        ::testing::ValuesIn(testData));
