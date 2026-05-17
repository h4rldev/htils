set quiet
set shell := ["bash", "-c"]

proj_name := "htils"
bin_name := "htils-test"

# Pretty Colors

red := "\\x1b[31m"
green := "\\x1b[32m"
yellow := "\\x1b[33m"
reset := "\\x1b[0m"

# Directories

include := "include"
src := "src"
htils_src := src + "/" + "htils"
test_src := src + "/" + "testing"
out := "out"
htils_out := out + "/" + "htils"
test_out := out + "/" + "testing"
bin := "bin"
lib := "lib"

# Flags

include_flags := '-I' + include
link_flags := ''
debug_shared_flags := '-ggdb -g -Og -fsanitize=address,undefined,leak -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-common -std=gnu11 -Wl,-rpath,.'
debug_compile_flags := debug_shared_flags + ' -Wall -Wextra -Wpedantic -Wno-unused-parameter'
debug_link_flags := debug_shared_flags + link_flags + ' -static-libasan -lhtils-debug -Llib'
release_shared_flags := '-O2 -std=gnu11 -Wl,-rpath,.'
release_compile_flags := release_shared_flags
release_link_flags := release_shared_flags + link_flags + '  -lhtils -Llib'
debug_static_link_flags := debug_link_flags + ' -static'
release_static_link_flags := release_link_flags + ' -static'

# Default justfile target (lists all available targets)
default:
    just --list

compile-htils type="debug" force="false" threads=num_cpus():
    #!/usr/bin/env bash
    shopt -s globstar

    [[ -d {{ htils_out }} ]] || mkdir -p {{ htils_out }}

    WILL_COMPILE=false

    for file in {{ htils_src }}/**/*.c; do
        if [[ $file -nt  {{ htils_out }}/$(basename "${file%.c}")-{{ type }}.o ]]; then
          WILL_COMPILE=true
        fi

        if ! [[ -f {{ htils_out }}/$(basename "${file%.c}")-{{ type }}.o ]]; then
          WILL_COMPILE=true
        fi
    done

    if [[ {{ force }} == "force" || {{ force }} == "true" ]]; then
        echo -e "Compile: Forcing"
        WILL_COMPILE=true
    fi

    if [[ $WILL_COMPILE == false ]]; then
        echo -e "Compile: Nothing to do"
        exit 0
    fi


    echo -e "Using {{ red }}{{ threads }}{{ reset }} threads"
    echo -e "Target: {{ green }}{{ type }}{{ reset }}"
    if [[ {{ type }} == "debug" ]]; then
        find {{ htils_src }} -name "*.c" -print0 | xargs -0 -P{{ threads }} -n1 \
            sh -c 'if [[ "$1" -nt "{{ htils_out }}/$(basename "${1%.c}")-debug.o" || {{ force }} == "true" || {{ force }} == "force" ]]; then echo -e "Compiling {{ green }}$1{{ reset }}..."; gcc {{ include_flags }} {{ debug_compile_flags }} -c "$1" -o "{{ htils_out }}/$(basename "${1%.c}")-debug.o"; fi' sh
    else
        find {{ htils_src }} -name "*.c" -print0 | xargs -0 -P{{ threads }} -n1 \
            sh -c 'if [[ "$1" -nt "{{ htils_out }}/$(basename "${1%.c}")-release.o" || {{ force }} == "true" || {{ force }} == "force" ]]; then echo -e "Compiling {{ green }}$1{{ reset }}..."; gcc {{ include_flags }} {{ release_compile_flags }} -c "$1" -o "{{ htils_out }}/$(basename "${1%.c}")-release.o"; fi' sh
    fi

    echo -e "Compile (htils): Compiling {{ green }}{{ type }}{{ reset }} complete"

assemble-htils type="debug" force="false" static="dynamic":
    #!/usr/bin/env bash
    shopt -s globstar

    [[ -d {{ htils_out }} ]] || just build-htils
    [[ -d {{ lib }} ]] || mkdir -p {{ lib }}

    WILL_ASSEMBLE=false

    if [[ {{ type }} == "debug" ]]; then
        for file in {{ htils_out }}/*-debug.o; do
            if [[ $file -nt {{ lib }}/libhtils-{{ type }}.a ]]; then
                WILL_ASSEMBLE=true
            fi
        done
    else
        if [[ {{ static }} == "static" || {{ static }} == "true" ]]; then
            for file in {{ htils_out }}/*-release.o; do
                if [[ $file -nt {{ lib }}/libhtils.a ]]; then
                    WILL_ASSEMBLE=true
                fi
            done
        else
            for file in {{ htils_out }}/*-release.o; do
                if [[ $file -nt {{ lib }}/libhtils.so ]]; then
                    WILL_ASSEMBLE=true
                fi
            done
        fi
    fi

    if [[ $WILL_ASSEMBLE == false ]]; then
        echo -e "Assemble (htils): Nothing to do"
        exit 0
    fi

    echo -e "Target: {{ green }}{{ type }}{{ reset }}"
    if [[ {{ type }} == "debug" ]]; then
        ar -rcs {{ lib }}/libhtils-{{ type }}.a {{ htils_out }}/*-{{ type }}.o
        ranlib {{ lib }}/libhtils-{{ type }}.a
    else
        if [[ {{ static }} == "dynamic" || {{ static }} == "false" ]]; then
            gcc -shared -o {{ lib }}/libhtils.so {{ htils_out }}/*-release.o -fuse-ld=mold
            strip {{ lib }}/libhtils.so
            cp {{ lib }}/libhtils.so {{ bin }}
        else
            ar -rcs {{ lib }}/libhtils.a {{ htils_out }}/*-{{ type }}.o
            strip --strip-debug {{ lib }}/libhtils.a
            ranlib {{ lib }}/libhtils.a
        fi
    fi

    echo -e "Assemble (htils): Assemble {{ green }}{{ type }}{{ reset }} complete"

compile-test type="debug" force="false" threads=num_cpus():
    #!/usr/bin/env bash
    shopt -s globstar

    [[ -d {{ test_out }} ]] || mkdir -p {{ test_out }}

    WILL_COMPILE=false

    for file in {{ test_src }}/**/*.c; do
        if [[ $file -nt  {{ test_out }}/$(basename "${file%.c}")-{{ type }}.o ]]; then
            WILL_COMPILE=true
        fi

        if ! [[ -f {{ test_out }}/$(basename "${file%.c}")-{{ type }}.o ]]; then
            WILL_COMPILE=true
        fi
    done

    if [[ {{ force }} == "force" || {{ force }} == "true" ]]; then
        echo -e "Compile (test): Forcing"
        WILL_COMPILE=true
    fi

    if [[ $WILL_COMPILE == false ]]; then
        echo -e "Compile (test): Nothing to do"
        exit 0
    fi


    echo -e "Using {{ red }}{{ threads }}{{ reset }} threads"
    echo -e "Target: {{ green }}{{ type }}{{ reset }}"
    if [[ {{ type }} == "debug" ]]; then
        find {{ test_src }} -name "*.c" -print0 | xargs -0 -P{{ threads }} -n1 \
            sh -c 'if [[ "$1" -nt "{{ test_out }}/$(basename "${1%.c}")-debug.o" || {{ force }} == "true" || {{ force }} == "force" ]]; then echo -e "Compiling {{ green }}$1{{ reset }}..."; gcc {{ include_flags }} {{ debug_compile_flags }} -c "$1" -o "{{ test_out }}/$(basename "${1%.c}")-debug.o"; fi' sh
    else
        find {{ test_src }} -name "*.c" -print0 | xargs -0 -P{{ threads }} -n1 \
            sh -c 'if [[ "$1" -nt "{{ test_out }}/$(basename "${1%.c}")-release.o" || {{ force }} == "true" || {{ force }} == "force" ]]; then echo -e "Compiling {{ green }}$1{{ reset }}..."; gcc {{ include_flags }} {{ release_compile_flags }} -c "$1" -o "{{ test_out }}/$(basename "${1%.c}")-release.o"; fi' sh
    fi

    echo -e "Compile (test): Compiling {{ green }}{{ type }}{{ reset }} complete"

link-test type="debug" static="dynamic":
    #!/usr/bin/env bash
    shopt -s globstar

    [[ -d {{ test_out }} ]] || just compile-test {{ type }}
    [[ -d {{ bin }} ]] || mkdir -p {{ bin }}

    WILL_LINK=false

    if [[ {{ static }} == "static" || {{ static }} == "true" ]]; then
        if [[ {{ type }} == "release" ]]; then
            [[ -f {{ lib }}/libhtils.a ]] || just build-htils {{ type }} static
            if [[ {{ lib }}/libhtils.a -nt {{ bin }}/{{ bin_name }}-static ]]; then
                WILL_LINK=true
            fi

            for file in {{ test_out }}/*-release.o; do
                if [[ $file -nt {{ bin }}/{{ bin_name }}-static ]]; then
                    WILL_LINK=true
                fi
            done
        else
            echo "Link (test): Can't have a static debug build, there's just no reason to."
        fi
    else
        if [[ {{ type }} == "debug" ]]; then
            [[ -f {{ lib }}/libhtils-debug.a ]] || just build-htils {{ type }}
            if [[ {{ lib }}/libhtils-debug.a -nt {{ bin }}/{{ bin_name }}-debug ]]; then
                WILL_LINK=true
            fi

            for file in {{ test_out }}/*-{{ type }}.o; do
                if [[ $file -nt {{ bin }}/{{ bin_name }}-{{ type }} ]]; then
                    WILL_LINK=true
                fi
            done
        else
            [[ -f {{ lib }}/libhtils.so ]] || just build-htils {{ type }}
            if [[ {{ lib }}/libhtils.so -nt {{ bin }}/{{ bin_name }} ]]; then
                WILL_LINK=true
            fi

            for file in {{ test_out }}/*-{{ type }}.o; do
                if [[ $file -nt {{ bin }}/{{ bin_name }} ]]; then
                    WILL_LINK=true
                fi
            done
        fi
    fi

    if [[ $WILL_LINK == false ]]; then
        echo -e "Link (test): Nothing to do"
        exit 0
    fi

    FILES=$(find {{ test_out }} -name "*-{{ type }}.o")

    if [[ {{ static }} == "dynamic" || {{ static }} == "false" ]]; then
        echo -e "Linking {{ green }}{{ type }}{{ reset }} dynamically"
        echo -e "Files to link: {{ green }}"
        printf "%s\n" "${FILES[@]}" | sed 's/^/    /'
        echo -e "{{ reset }}"

        if [[ {{ type }} == "debug" ]]; then
            gcc {{ test_out }}/*-debug.o {{ debug_link_flags }} -o {{ bin }}/{{ bin_name }}-debug -fuse-ld=mold
        else
            gcc {{ test_out }}/*-release.o {{ release_link_flags }} -o {{ bin }}/{{ bin_name }} -fuse-ld=mold
            strip --strip-all {{ bin }}/{{ bin_name }}
        fi
        echo -e "Link (test): Linking {{ green }}{{ type }}{{ reset }} dynamically complete"
    else
        echo -e "Linking {{ green }}{{ type }}{{ reset }} statically"
        echo -e "Files to link: {{ green }}"
        printf "%s\n" "${FILES[@]}" | sed 's/^/    /'
        echo -e "{{ reset }}"
        gcc {{ test_out }}/*-release.o {{ release_static_link_flags }} -o {{ bin }}/{{ bin_name }}-static -fuse-ld=mold
        strip --strip-all {{ bin }}/{{ bin_name }}-static
        echo -e "Link (test): Linking {{ green }}{{ type }}{{ reset }} statically complete"
    fi

build-htils type="debug" force="false" static="dynamic" threads=num_cpus():
    just compile-htils {{ type }} {{ force }} {{ threads }}
    just assemble-htils {{ type }} {{ force }} {{ static }}

build-test type="debug" force="false" static="dynamic" threads=num_cpus():
    just build-htils {{ type }} {{ force }} {{ static }}
    just compile-test {{ type }} {{ force }} {{ threads }}
    just link-test {{ type }} {{ static }}

build-test-static force="false" threads=num_cpus():
    just build-test release {{ force }} {{ threads }} static

debug force="false" threads=num_cpus():
    just build-htils debug {{ force }} {{ threads }}

release force="false" threads=num_cpus():
    just build-htils release {{ force }} {{ threads }}

release-static force="false" threads=num_cpus():
    just build-htils-static {{ force }} {{ threads }}

debug-test force="false" threads=num_cpus():
    just build-test debug {{ force }} {{ threads }}

release-test force="false" threads=num_cpus():
    just build-test release {{ force }} {{ threads }}

release_static force="false" threads=num_cpus():
    just build_static {{ force }} {{ threads }}

clean:
    [[ ! -d {{ lib }} ]] || rm -rf {{ lib }}
    [[ ! -d {{ out }} ]] || rm -rf {{ out }}
    [[ ! -d {{ bin }} ]] || rm -rf {{ bin }}

__bear-compile:
    just compile-htils debug force
    just compile-test debug force

bear:
    bear -- just __bear-compile
    sed -i 's|"/nix/store/[^"]*gcc[^"]*|\"gcc|g' compile_commands.json
