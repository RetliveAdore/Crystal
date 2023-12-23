if ls CMakeLists.txt;then
    read -p "clear to rebuild? [y/n] (Default: y):" para
    para=${para:-"y"}
    case $para in
        [y])
            echo "clearing..."
            if ls build/;then
                rm -rf build/
            fi
            mkdir build
            echo "building"
            cmake -S . -B build/
            cd build
            make
            ;;
        [n])
            echo "building"
            cd build/
            make
            ;;
        *)
            echo "invalid option"
    esac
else
    echo "You need a CMakeLists.txt to make it"
fi
