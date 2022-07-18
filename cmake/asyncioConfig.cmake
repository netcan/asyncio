get_filename_component(ASYNCIO_INCLUDES "${CMAKE_CURRENT_LIST_DIR}/../include/" ABSOLUTE)

set(asyncio_INCLUDE_DIRS "${ASYNCIO_INCLUDES}")
set(asyncio_LIBRARIES asyncio)
set(asyncio_LIBRARY_DIRS "lib")
