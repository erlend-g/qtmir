include_directories(SYSTEM ${MIRSERVER_INCLUDE_DIRS} ${MIRRENDERERGLDEV_INCLUDE_DIRS} ${MIROIL_INCLUDE_DIRS} ${MIRPLATFORM_INCLUDE_DIRS})

add_library(miral-prototypes OBJECT
    edid.cpp edid.h
    mirbuffer.cpp mirbuffer.h
)
