#!/bin/sh


usage()
{
  echo "Usage: [cpu] [board]"
  echo "mkuboot.sh [orion]      [dorado | dorado2 | virgo| vela | libra | leo | all]"
  echo "mkuboot.sh [equuleus]   [8182 | 8133 | 8132 | all]"
}

# $1: cpu (str9100)
# $2: board
make_for_board()
{
# arch/arm/configs/str9100_dorado_ramimage_defconfig
  echo "in make_for_board"
  echo "cpu: $1"
  echo "board: $2"

#  make clean
#  make distclean
#  echo make star_$1_$2_config
#  make star_$1_$2_config
#  make 
#  cp -v u-boot.bin /opt/star/output/u-boot.$1.$2.bin
#  cp arch/arm/configs/$1_$2_ramimage_defconfig .config && make oldconfig && make clean && ./mkbootp.sh
  ./mkbootp.sh
}

make_all_orion()
{
  make_for_board orion dorado
  make_for_board orion dorado2
  make_for_board orion virgo
  make_for_board orion vela
  make_for_board orion leo
  make_for_board orion libra
}

make_all_equuleus()
{
  make_for_board equuleus 8133
  make_for_board equuleus 8132
  make_for_board equuleus 8182
}

if [ "$1" = "orion" ] ; then
  echo "make orion"

  if [ "$2" = "dorado" ] || [ "$2" = "dorado2" ] || [ "$2" = "virgo" ] || [ "$2" = "leo" ] || [ "$2" = "libra" ] || [ "$2" = "vela" ] || [ "$2" = "all" ] ; then
    if [ "$2" = "all" ] ; then
      echo "make_all_orion"
      make_all_orion
    else
      make_for_board str9100 $2
    fi
  fi
  exit 0
fi

if [ "$1" = "equuleus" ]  ; then
  echo "make equuleus"
  if [ "$2" = "8133" ] || [ "$2" = "8132" ] || [ "$2" = "8182" ] || [ "$2" = "all" ] ; then
    if [ "$2" = "all" ] ; then
      make_all_equuleus
    else
      make_for_board $1 $2
    fi
  fi
  exit 0
fi

if [ "$1" = "" ] ; then
  usage
fi

exit 0

case "$1" in
	star_equuleus_8182_config|8182)
		echo make_for_board 8182
		;;
	star_equuleus_8133_config|8133)
		echo make_for_board 8133
		;;
	star_equuleus_8132_config|8132)
		echo make_for_board 8132
		;;
	all)
		echo make_for_board 8182
		echo make_for_board 8133
		;;
	*|"")
		usage;
esac


