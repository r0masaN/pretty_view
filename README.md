# collections_io_streams
Simple C++ library that complements 'std' by adding custom overloadings for '&lt;&lt;' and '>>' operators in iostreams for standart collections ('vector', 'map', etc.)

## Usage for 'ostream'
1. depending on collection type, '#define \[COLLECTION\]_PREFIX/DELIMITER/POSTFIX "..."' if you need custom prefix, delimiter or postfix (by defauls it's "\[", ", " and "\]" for all collections and "{", ": " and "}" for 'pair')
2. #include needed header ('\[collection\]_ostream.hpp')
3. easily print selected collection with any 'ostream' with '<<' like default types
 
