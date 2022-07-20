build_lib(
  LIBNAME blood-voyager-s
  
  SOURCE_FILES Bloodvessel.cc
    Bloodstream.cc
    Nanobot.cc
    Bloodcircuit.cc
    PrintNanobots.cc
   
  HEADER_FILES Bloodvessel.h
    Bloodstream.h
    Nanobot.h
    Bloodcircuit.h
    PrintNanobots.h
  
  LIBRARIES_TO_LINK
    ${libinternet}
    ${libcore}
    ${libmobility}
    ${libwifi}
    ${libnetwork}
)