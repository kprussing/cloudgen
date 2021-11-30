foreach(script generate_cases_varying_windshear.sh
               generate_nomixing_varying_windshear.sh
               )
    file(READ ${script} text)
    string(REPLACE ../cloudgen cloudgen _text "${text}")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${script} "${_text}")
endforeach()

install(FILES README
              19990624.dat
              19990624_nomixing.dat
              19990717.dat
              19990717_nomixing.dat
              19990827.dat
              19990827_nomixing.dat
              19991227.dat
              19991227_nomixing.dat
              cirrus.dat
              iwc.nc
              cirrus_with_effective_radius.dat
              stratocumulus.dat
              lwc.nc
              "${CMAKE_CURRENT_BINARY_DIR}/generate_cases_varying_windshear.sh"
              "${CMAKE_CURRENT_BINARY_DIR}/generate_nomixing_varying_windshear.sh"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cloudgen
)
