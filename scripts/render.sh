#!/bin/sh

# set project init variables
ProjectName="MyProjectName"
Description="Description of project"
Author="Author"
Today=$(date '+%Y%m%d')
Version="0.1"
Machine="$(uname -s)"

ReplaceItems="\
s,{{ProjectName}},${ProjectName},g;\
s,{{Description}},${Description},g;\
s,{{Author}},${Author},g;\
s,{{Today}},${Today},g;\
s,{{Version}},${Version},g;\
" 

echo "Rendering file templates"
case "${Machine}" in
    Linux*)  
        find . -type d -name .git -prune -o -type f -print0 | xargs -0 sed -i "${ReplaceItems}"
        ;;
    Darwin*) 
        find . -type d -name .git -prune -o -type f -print0 | LC_ALL=C xargs -0 sed -i "" "${ReplaceItems}"
        ;;
    *)
esac

# rename files
echo "Rendering file names"
mv "include/ProjectName.h" "include/${ProjectName}.h"
mv "src/ProjectName.cpp" "src/${ProjectName}.cpp"
mv "tests/ProjectNameTest.cpp" "tests/${ProjectName}Test.cpp"
mv "cmake/ProjectNameConfig.cmake" "cmake/${ProjectName}Config.cmake"

echo "All Done"

