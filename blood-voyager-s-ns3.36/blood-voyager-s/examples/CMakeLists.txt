build_lib_example(
  NAME start-bvs
  SOURCE_FILES start-blood-voyager-s
    
  LIBRARIES_TO_LINK 
    ${libblood-voyager-s}
    ${libinternet}
    ${libcore}
    ${libmobility}
    ${libwifi}
    ${libnetwork}
)
