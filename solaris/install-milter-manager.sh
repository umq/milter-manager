#! /usr/bin/bash --noprofile

set -e

source ./environment.sh
source ./functions.sh

install_milter_manager()
{
    local base="milter-manager"
    local log="${BUILDS}/${base}.build.log"
    local build_dir="${base_dir}/../"

    mkdir -p "${BUILDS}"

    if test -f "${build_dir}/Makefile"; then
	echo "$(time_stamp): Cleaning ${base}..."
	run ${MAKE} -C "${build_dir}" clean > "${log}"
	echo "$(time_stamp): done."
    fi

    echo "$(time_stamp): Configuring ${base}..."
    (
        cd "${build_dir}"
        run bash ./autogen.sh
        run ./configure --prefix $PREFIX --enable-ruby-milter
    ) > "${log}"
    echo "$(time_stamp): done."

    echo "$(time_stamp): Building ${base}..."
    run ${MAKE} -C "${build_dir}" > "${log}"
    echo "$(time_stamp): done."

    build_pkg "milter-manager" "${build_dir}"
    install_pkg "milter-manager"
}

echo "$(time_stamp): Installing milter manager package..."
install_milter_manager
echo "$(time_stamp): done."
