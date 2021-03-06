#!/usr/bin/bash --noprofile

set -e

source ./environment.sh
source ./functions.sh

build_other=$1
build_mysql=$2
build_ruby=$3

echo "$(time_stamp): Installing libraries..."

gnu_org="http://ftp.gnu.org/pub/gnu"
gnome_org="http://ftp.gnome.org/pub/gnome/sources"
ruby_lang_org="ftp://ftp.ruby-lang.org/pub/ruby"
mysql_downloads="http://ftp.jaist.ac.jp/pub/mysql/Downloads"

install_other_packages()
{
    local build_flag=$1
    if test $build_flag = false; then
        return
    fi
    if test "$USE_OPEN_CSW" != "yes"; then
        CC="$CC -m64" \
	        install_package ${gnu_org}/gettext/gettext-0.18.1.1.tar.gz \
	        --enable-relocatable --without-git
        install_package ${gnu_org}/libiconv/libiconv-1.13.1.tar.gz
        yes | run_pfexec /usr/sbin/pkgrm "${PKG_PREFIX}gettext" > \
	        "${PKGS}/${PKG_PREFIX}gettext-uninstall.log" 2>&1
        CC="$CC -m64" \
	        install_package ${gnu_org}/gettext/gettext-0.18.1.1.tar.gz \
	        --enable-relocatable --without-git
        LDFLAGS="-lsocket -lnsl $LDFLAGS" \
	        install_package ${gnome_org}/glib/2.28/glib-2.28.1.tar.bz2 \
	        --with-libiconv=gnu
    fi
}

install_mysql(){
    local build_flag=$1
    if test $build_flag = false; then
        return
    fi
    CC="$CC -m64" \
        install_package ${mysql_downloads}/MySQL-5.5/mysql-5.5.9.tar.gz \
        -DWITH_UNIT_TESTS=0
}

install_ruby(){
    local build_flag=$1
    if test $build_flag = false; then
        return
    fi
    ruby_base="ruby-1.9.2-p180"
    CC="$CC -m64 -ggdb3" \
        install_package ${ruby_lang_org}/1.9/${ruby_base}.tar.bz2 \
        --disable-install-doc --enable-shared --enable-load-relative

    dest_prefix=${PKG_DESTDIR}${PREFIX}
    gem_prefix=${dest_prefix}/lib/ruby/gems/1.9.1
    gem_options="--no-ri --no-rdoc"
    gem_options="${gem_options} --install-dir ${gem_prefix}"
    gem_options="${gem_options} --bindir ${dest_prefix}/bin"
    gems="bundler activerecord mail pkg-config mysql2"
    if test -d "$GEMS_CACHE"; then
        gem_options="${gem_options} --local"
        gem_command="gem install ${gems} ${gem_options}"
        CC="$CC -m64" \
            build_pkg "${ruby_base}" "${BUILDS}/${ruby_base}" \
            "cd ${GEMS_CACHE}" "${gem_command}"
    else
        gem_command="gem install ${gems} ${gem_options}"
        CC="$CC -m64" \
            build_pkg "${ruby_base}" "${BUILDS}/${ruby_base}" \
            "${gem_command}"
    fi
    yes | run_pfexec /usr/sbin/pkgrm "${PKG_PREFIX}ruby" > \
        "${PKGS}/${PKG_PREFIX}ruby-uninstall.log" 2>&1
    install_pkg "${ruby_base}"
}

install_cutter(){
    local build_flag=$1
    if test $build_flag = false; then
        return
    fi
    install_package http://downloads.sourceforge.net/cutter/cutter-1.1.7.tar.gz
}

install_other_packages $build_other
install_mysql $build_mysql
install_ruby $build_ruby
install_cutter $build_other

echo "$(time_stamp): done."
