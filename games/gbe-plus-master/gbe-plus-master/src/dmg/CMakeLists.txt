set(SRCS
	apu.cpp
	core.cpp
	gamepad.cpp
	lcd.cpp
	mbc1.cpp
	mbc2.cpp
	mbc3.cpp
	mbc5.cpp
	mbc6.cpp
	mbc7.cpp
	huc1.cpp
	huc3.cpp
	mmm01.cpp
	camera.cpp
	tama5.cpp
	mmu.cpp
	opengl.cpp
	z80.cpp
	sio.cpp
	infrared.cpp
	dmg07.cpp
	gbma.cpp
	debug.cpp
	)

set(HEADERS
	apu.h
	apu_data.h
	common.h
	core.h
	gamepad.h
	lcd.h
	lcd_data.h
	mmu.h
	z80.h
	sio.h
	sio_data.h
	)

add_library(dmg STATIC ${SRCS} ${HEADERS})

target_link_libraries(dmg common ${SDL2_LIBRARY})

if (LINK_CABLE)
    target_link_libraries(dmg ${SDL2NET_LIBRARY})
endif()

if (USE_OGL)
    target_link_libraries(dmg ${OPENGL_gl_LIBRARY})
endif()

if (WIN32)
    target_link_libraries(dmg ${GLEW_LIBRARY})
endif()
