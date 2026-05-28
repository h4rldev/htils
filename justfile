set quiet
set shell := ["bash", "-c"]

proj_name := "htils"
htils_bin_name := "htils-test"
h2otils_bin_name := "h2otils-test"

# Pretty Colors

red := "\\x1b[31m"
green := "\\x1b[32m"
yellow := "\\x1b[33m"
reset := "\\x1b[0m"

# Directories

include := "include"
src := "src"
out := "out"
bin := "bin"
lib := "lib"

htils_src := src + "/" + "htils"
htils_test_src := src + "/" + "testing" + "/" + "htils"

htils_out := out + "/" + "htils"
htils_test_out := out + "/" + "testing" + "/" + "htils"

h2otils_src := src + "/" + "h2otils"
h2otils_test_src := src + "/" + "testing" + "/" + "h2otils"

h2otils_out := out + "/" + "h2otils"
h2otils_test_out := out + "/" + "testing" + "/" + "h2otils"

# Flags

h2otils_link_flags := '-lh2o'

include_flags := '-I' + include
link_flags := ''
debug_shared_flags := '-ggdb -g -Og -fsanitize=address,undefined,leak -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-common -std=gnu11 -Wl,-rpath,.'
debug_compile_flags := debug_shared_flags + ' -Wall -Wextra -Wpedantic -Wno-unused-parameter'
htils_debug_link_flags := debug_shared_flags + link_flags + ' -static-libasan -lhtils-debug -Llib'
h2otils_debug_link_flags := htils_debug_link_flags + ' -lh2otils-debug -lh2o'

release_shared_flags := '-O2 -std=gnu11 -Wl,-rpath,.'
release_compile_flags := release_shared_flags
htils_release_link_flags := release_shared_flags + link_flags + '  -lhtils -Llib'
h2otils_release_link_flags := htils_release_link_flags + ' -lh2otils -lh2o'

htils_debug_static_link_flags := htils_debug_link_flags + ' -static  -fPIC'
h2otils_debug_static_link_flags := htils_debug_static_link_flags + ' -static  -fPIC'

htils_release_static_link_flags := htils_release_link_flags + ' -static -fPIC'
h2otils_release_static_link_flags := h2otils_release_link_flags + ' -static -fPIC'

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
        echo -e "Compile(htils): Forcing"
        WILL_COMPILE=true
    fi

    if [[ $WILL_COMPILE == false ]]; then
        echo -e "Compile(htils): Nothing to do"
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

    [[ -d {{ htils_out }} ]] || just compile-htils
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

compile-h2otils type="debug" force="false" threads=num_cpus():
    #!/usr/bin/env bash
    shopt -s globstar

    [[ -d {{ h2otils_out }} ]] || mkdir -p {{ h2otils_out }}

    WILL_COMPILE=false

    for file in {{ h2otils_src }}/**/*.c; do
        if [[ $file -nt  {{ h2otils_out }}/$(basename "${file%.c}")-{{ type }}.o ]]; then
          WILL_COMPILE=true
        fi

        if ! [[ -f {{ h2otils_out }}/$(basename "${file%.c}")-{{ type }}.o ]]; then
          WILL_COMPILE=true
        fi
    done

    if [[ {{ force }} == "force" || {{ force }} == "true" ]]; then
        echo -e "Compile(h2otils): Forcing"
        WILL_COMPILE=true
    fi

    if [[ $WILL_COMPILE == false ]]; then
        echo -e "Compile(h2otils): Nothing to do"
        exit 0
    fi


    echo -e "Using {{ red }}{{ threads }}{{ reset }} threads"
    echo -e "Target: {{ green }}{{ type }}{{ reset }}"
    if [[ {{ type }} == "debug" ]]; then
        find {{ h2otils_src }} -name "*.c" -print0 | xargs -0 -P{{ threads }} -n1 \
            sh -c 'if [[ "$1" -nt "{{ h2otils_out }}/$(basename "${1%.c}")-debug.o" || {{ force }} == "true" || {{ force }} == "force" ]]; then echo -e "Compiling {{ green }}$1{{ reset }}..."; gcc {{ include_flags }} {{ debug_compile_flags }} -c "$1" -o "{{ h2otils_out }}/$(basename "${1%.c}")-debug.o"; fi' sh
    else
        find {{ h2otils_src }} -name "*.c" -print0 | xargs -0 -P{{ threads }} -n1 \
            sh -c 'if [[ "$1" -nt "{{ h2otils_out }}/$(basename "${1%.c}")-release.o" || {{ force }} == "true" || {{ force }} == "force" ]]; then echo -e "Compiling {{ green }}$1{{ reset }}..."; gcc {{ include_flags }} {{ release_compile_flags }} -c "$1" -o "{{ h2otils_out }}/$(basename "${1%.c}")-release.o"; fi' sh
    fi

    echo -e "Compile (h2otils): Compiling {{ green }}{{ type }}{{ reset }} complete"

assemble-h2otils type="debug" force="false" static="dynamic":
    #!/usr/bin/env bash
    shopt -s globstar

    [[ -d {{ h2otils_out }} ]] || just compile-h2otils
    [[ -d {{ lib }} ]] || mkdir -p {{ lib }}

    WILL_ASSEMBLE=false

    if [[ {{ type }} == "debug" ]]; then
        for file in {{ h2otils_out }}/*-debug.o; do
            if [[ $file -nt {{ lib }}/libh2otils-{{ type }}.a ]]; then
                WILL_ASSEMBLE=true
            fi
        done
    else
        if [[ {{ static }} == "static" || {{ static }} == "true" ]]; then
            for file in {{ h2otils_out }}/*-release.o; do
                if [[ $file -nt {{ lib }}/libh2otils.a ]]; then
                    WILL_ASSEMBLE=true
                fi
            done
        else
            for file in {{ h2otils_out }}/*-release.o; do
                if [[ $file -nt {{ lib }}/libh2otils.so ]]; then
                    WILL_ASSEMBLE=true
                fi
            done
        fi
    fi

    if [[ $WILL_ASSEMBLE == false ]]; then
        echo -e "Assemble (h2otils): Nothing to do"
        exit 0
    fi

    echo -e "Target: {{ green }}{{ type }}{{ reset }}"
    if [[ {{ type }} == "debug" ]]; then
        ar -rcs {{ lib }}/libh2otils-{{ type }}.a {{ h2otils_out }}/*-{{ type }}.o
        ranlib {{ lib }}/libh2otils-{{ type }}.a
    else
        if [[ {{ static }} == "dynamic" || {{ static }} == "false" ]]; then
            gcc -shared -o {{ lib }}/lib2ohtils.so {{ h2otils_out }}/*-release.o -fuse-ld=mold
            strip {{ lib }}/libh2otils.so
            cp {{ lib }}/libh2otils.so {{ bin }}
        else
            ar -rcs {{ lib }}/libh2otils.a {{ h2otils_out }}/*-{{ type }}.o
            strip --strip-debug {{ lib }}/libh2otils.a
            ranlib {{ lib }}/libh2otils.a
        fi
    fi

    echo -e "Assemble (ht2otils): Assemble {{ green }}{{ type }}{{ reset }} complete"

compile-htils-test type="debug" force="false" threads=num_cpus():
    #!/usr/bin/env bash
    shopt -s globstar

    [[ -d {{ htils_test_out }} ]] || mkdir -p {{ htils_test_out }}

    WILL_COMPILE=false

    for file in {{ htils_test_src }}/**/*.c; do
        if [[ $file -nt  {{ htils_test_out }}/$(basename "${file%.c}")-{{ type }}.o ]]; then
            WILL_COMPILE=true
        fi

        if ! [[ -f {{ htils_test_out }}/$(basename "${file%.c}")-{{ type }}.o ]]; then
            WILL_COMPILE=true
        fi
    done

    if [[ {{ force }} == "force" || {{ force }} == "true" ]]; then
        echo -e "Compile (htils-test): Forcing"
        WILL_COMPILE=true
    fi

    if [[ $WILL_COMPILE == false ]]; then
        echo -e "Compile (htils-test): Nothing to do"
        exit 0
    fi


    echo -e "Using {{ red }}{{ threads }}{{ reset }} threads"
    echo -e "Target: {{ green }}{{ type }}{{ reset }}"
    if [[ {{ type }} == "debug" ]]; then
        find {{ htils_test_src }} -name "*.c" -print0 | xargs -0 -P{{ threads }} -n1 \
            sh -c 'if [[ "$1" -nt "{{ htils_test_out }}/$(basename "${1%.c}")-debug.o" || {{ force }} == "true" || {{ force }} == "force" ]]; then echo -e "Compiling {{ green }}$1{{ reset }}..."; gcc {{ include_flags }} {{ debug_compile_flags }} -c "$1" -o "{{ htils_test_out }}/$(basename "${1%.c}")-debug.o"; fi' sh
    else
        find {{ htils_test_src }} -name "*.c" -print0 | xargs -0 -P{{ threads }} -n1 \
            sh -c 'if [[ "$1" -nt "{{ htils_test_out }}/$(basename "${1%.c}")-release.o" || {{ force }} == "true" || {{ force }} == "force" ]]; then echo -e "Compiling {{ green }}$1{{ reset }}..."; gcc {{ include_flags }} {{ release_compile_flags }} -c "$1" -o "{{ htils_test_out }}/$(basename "${1%.c}")-release.o"; fi' sh
    fi

    echo -e "Compile (htils-test): Compiling {{ green }}{{ type }}{{ reset }} complete"

link-htils-test type="debug" static="dynamic":
    #!/usr/bin/env bash
    shopt -s globstar

    [[ -d {{ htils_test_out }} ]] || just compile-test {{ type }}
    [[ -d {{ bin }} ]] || mkdir -p {{ bin }}

    WILL_LINK=false

    if [[ {{ static }} == "static" || {{ static }} == "true" ]]; then
        if [[ {{ type }} == "release" ]]; then
            [[ -f {{ lib }}/libhtils.a ]] || just build-htils {{ type }} static
            if [[ {{ lib }}/libhtils.a -nt {{ bin }}/{{ htils_bin_name }}-static ]]; then
                WILL_LINK=true
            fi

            for file in {{ htils_test_out }}/*-release.o; do
                if [[ $file -nt {{ bin }}/{{ htils_bin_name }}-static ]]; then
                    WILL_LINK=true
                fi
            done
        else
            echo "Link (htils-test): Can't have a static debug build, there's just no reason to."
        fi
    else
        if [[ {{ type }} == "debug" ]]; then
            [[ -f {{ lib }}/libhtils-debug.a ]] || just build-htils {{ type }}
            if [[ {{ lib }}/libhtils-debug.a -nt {{ bin }}/{{ htils_bin_name }}-debug ]]; then
                WILL_LINK=true
            fi

            for file in {{ htils_test_out }}/*-{{ type }}.o; do
                if [[ $file -nt {{ bin }}/{{ htils_bin_name }}-{{ type }} ]]; then
                    WILL_LINK=true
                fi
            done
        else
            [[ -f {{ lib }}/libhtils.so ]] || just build-htils {{ type }}
            if [[ {{ lib }}/libhtils.so -nt {{ bin }}/{{ htils_bin_name }} ]]; then
                WILL_LINK=true
            fi

            for file in {{ htils_test_out }}/*-{{ type }}.o; do
                if [[ $file -nt {{ bin }}/{{ htils_bin_name }} ]]; then
                    WILL_LINK=true
                fi
            done
        fi
    fi

    if [[ $WILL_LINK == false ]]; then
        echo -e "Link (htils-test): Nothing to do"
        exit 0
    fi

    FILES=$(find {{ htils_test_out }} -name "*-{{ type }}.o")

    if [[ {{ static }} == "dynamic" || {{ static }} == "false" ]]; then
        echo -e "Linking {{ green }}{{ type }}{{ reset }} dynamically"
        echo -e "Files to link: {{ green }}"
        printf "%s\n" "${FILES[@]}" | sed 's/^/    /'
        echo -e "{{ reset }}"

        if [[ {{ type }} == "debug" ]]; then
            gcc {{ htils_test_out }}/*-debug.o {{ htils_debug_link_flags }} -o {{ bin }}/{{ htils_bin_name }}-debug -fuse-ld=mold
        else
            gcc {{ htils_test_out }}/*-release.o {{ htils_release_link_flags }} -o {{ bin }}/{{ htils_bin_name }} -fuse-ld=mold
            strip --strip-all {{ bin }}/{{ htils_bin_name }}
        fi
        echo -e "Link (htils-test): Linking {{ green }}{{ type }}{{ reset }} dynamically complete"
    else
        echo -e "Linking {{ green }}{{ type }}{{ reset }} statically"
        echo -e "Files to link: {{ green }}"
        printf "%s\n" "${FILES[@]}" | sed 's/^/    /'
        echo -e "{{ reset }}"
        gcc {{ htils_test_out }}/*-release.o {{ htils_release_static_link_flags }} -o {{ bin }}/{{ htils_bin_name }}-static -fuse-ld=mold
        strip --strip-all {{ bin }}/{{ htils_bin_name }}-static
        echo -e "Link (htils-test): Linking {{ green }}{{ type }}{{ reset }} statically complete"
    fi

compile-h2otils-test type="debug" force="false" threads=num_cpus():
    #!/usr/bin/env bash
    shopt -s globstar

    [[ -d {{ h2otils_test_out }} ]] || mkdir -p {{ h2otils_test_out }}

    WILL_COMPILE=false

    for file in {{ h2otils_test_src }}/**/*.c; do
        if [[ $file -nt  {{ h2otils_test_out }}/$(basename "${file%.c}")-{{ type }}.o ]]; then
            WILL_COMPILE=true
        fi

        if ! [[ -f {{ h2otils_test_out }}/$(basename "${file%.c}")-{{ type }}.o ]]; then
            WILL_COMPILE=true
        fi
    done

    if [[ {{ force }} == "force" || {{ force }} == "true" ]]; then
        echo -e "Compile (h2otils-test): Forcing"
        WILL_COMPILE=true
    fi

    if [[ $WILL_COMPILE == false ]]; then
        echo -e "Compile (h2otils-test): Nothing to do"
        exit 0
    fi


    echo -e "Using {{ red }}{{ threads }}{{ reset }} threads"
    echo -e "Target: {{ green }}{{ type }}{{ reset }}"
    if [[ {{ type }} == "debug" ]]; then
        find {{ h2otils_test_src }} -name "*.c" -print0 | xargs -0 -P{{ threads }} -n1 \
            sh -c 'if [[ "$1" -nt "{{ h2otils_test_out }}/$(basename "${1%.c}")-debug.o" || {{ force }} == "true" || {{ force }} == "force" ]]; then echo -e "Compiling {{ green }}$1{{ reset }}..."; gcc {{ include_flags }} {{ debug_compile_flags }} -c "$1" -o "{{ h2otils_test_out }}/$(basename "${1%.c}")-debug.o"; fi' sh
    else
        find {{ h2otils_test_src }} -name "*.c" -print0 | xargs -0 -P{{ threads }} -n1 \
            sh -c 'if [[ "$1" -nt "{{ h2otils_test_out }}/$(basename "${1%.c}")-release.o" || {{ force }} == "true" || {{ force }} == "force" ]]; then echo -e "Compiling {{ green }}$1{{ reset }}..."; gcc {{ include_flags }} {{ release_compile_flags }} -c "$1" -o "{{ h2otils_test_out }}/$(basename "${1%.c}")-release.o"; fi' sh
    fi

    echo -e "Compile (h2otils-test): Compiling {{ green }}{{ type }}{{ reset }} complete"

link-h2otils-test type="debug" static="dynamic":
    #!/usr/bin/env bash
    shopt -s globstar

    [[ -d {{ h2otils_test_out }} ]] || just compile-h2otils-test {{ type }}
    [[ -d {{ bin }} ]] || mkdir -p {{ bin }}

    WILL_LINK=false

    if [[ {{ static }} == "static" || {{ static }} == "true" ]]; then
        if [[ {{ type }} == "release" ]]; then
            [[ -f {{ lib }}/libh2otils.a ]] || just build-h2otils {{ type }} static
            [[ -f {{ lib }}/libhtils.a ]] || just build-htils {{ type }} static

            if [[ {{ lib }}/libh2otils.a -nt {{ bin }}/{{ h2otils_bin_name }}-static ]]; then
                WILL_LINK=true
            fi

            if [[ {{ lib }}/libhtils.a -nt {{ bin }}/{{ h2otils_bin_name }}-static ]]; then
              WILL_LINK=true
            fi

            for file in {{ h2otils_test_out }}/*-release.o; do
                if [[ $file -nt {{ bin }}/{{ h2otils_bin_name }}-static ]]; then
                    WILL_LINK=true
                fi
            done
        else
            echo "Link (h2otils-test): Can't have a static debug build, there's just no reason to."
        fi
    else
        if [[ {{ type }} == "debug" ]]; then

            [[ -f {{ lib }}/libh2otils-debug.a ]] || just build-h2otils {{ type }}
            [[ -f {{ lib }}/libhtils-debug.a ]] || just build-htils {{ type }}
            if [[ {{ lib }}/libh2otils-debug.a -nt {{ bin }}/{{ h2otils_bin_name }}-debug ]]; then
              WILL_LINK=true
            fi

            if [[ {{ lib }}/libhtils-debug.a -nt {{ bin }}/{{ h2otils_bin_name }}-debug ]]; then
                WILL_LINK=true
            fi

            for file in {{ h2otils_test_out }}/*-{{ type }}.o; do
                if [[ $file -nt {{ bin }}/{{ h2otils_bin_name }}-{{ type }} ]]; then
                    WILL_LINK=true
                fi
            done
        else
            [[ -f {{ lib }}/libh2otils.so ]] || just build-h2otils {{ type }}
            [[ -f {{ lib }}/libhtils.so ]] || just build-htils {{ type }}
            if [[ {{ lib }}/libh2otils.so -nt {{ bin }}/{{ h2otils_bin_name }} ]]; then
                WILL_LINK=true
            fi

            if [[ {{ lib }}/libhtils.so -nt {{ bin }}/{{ h2otils_bin_name }} ]]; then
                WILL_LINK=true
            fi

            for file in {{ h2otils_test_out }}/*-{{ type }}.o; do
                if [[ $file -nt {{ bin }}/{{ h2otils_bin_name }} ]]; then
                    WILL_LINK=true
                fi
            done
        fi
    fi

    if [[ $WILL_LINK == false ]]; then
        echo -e "Link (h2otils-test): Nothing to do"
        exit 0
    fi

    FILES=$(find {{ h2otils_test_out }} -name "*-{{ type }}.o")

    if [[ {{ static }} == "dynamic" || {{ static }} == "false" ]]; then
        echo -e "Linking {{ green }}{{ type }}{{ reset }} dynamically"
        echo -e "Files to link: {{ green }}"
        printf "%s\n" "${FILES[@]}" | sed 's/^/    /'
        echo -e "{{ reset }}"

        if [[ {{ type }} == "debug" ]]; then
            gcc {{ h2otils_test_out }}/*-debug.o {{ h2otils_debug_link_flags }} -o {{ bin }}/{{ h2otils_bin_name }}-debug -fuse-ld=mold
        else
            gcc {{ h2otils_test_out }}/*-release.o {{ h2otils_release_link_flags }} -o {{ bin }}/{{ h2otils_bin_name }} -fuse-ld=mold
            strip --strip-all {{ bin }}/{{ h2otils_bin_name }}
        fi
        echo -e "Link (h2otils-test): Linking {{ green }}{{ type }}{{ reset }} dynamically complete"
    else
        echo -e "Linking {{ green }}{{ type }}{{ reset }} statically"
        echo -e "Files to link: {{ green }}"
        printf "%s\n" "${FILES[@]}" | sed 's/^/    /'
        echo -e "{{ reset }}"
        gcc {{ htils_test_out }}/*-release.o {{ h2otils_release_static_link_flags }} -o {{ bin }}/{{ h2otils_bin_name }}-static -fuse-ld=mold
        strip --strip-all {{ bin }}/{{ h2otils_bin_name }}-static
        echo -e "Link (h2otils-test): Linking {{ green }}{{ type }}{{ reset }} statically complete"
    fi

build-htils type="debug" force="false" static="dynamic" threads=num_cpus():
    just compile-htils {{ type }} {{ force }} {{ threads }}
    just assemble-htils {{ type }} {{ force }} {{ static }}

build-h2otils type="debug" force="false" static="dynamic" threads=num_cpus():
    just compile-h2otils {{ type }} {{ force }} {{ threads }}
    just assemble-h2otils {{ type }} {{ force }} {{ static }}

build-htils-test type="debug" force="false" static="dynamic" threads=num_cpus():
    just build-htils {{ type }} {{ force }} {{ static }}
    just compile-htils-test {{ type }} {{ force }} {{ threads }}
    just link-htils-test {{ type }} {{ static }}

build-h2otils-test type="debug" force="false" static="dynamic" threads=num_cpus():
    just build-h2otils {{ type }} {{ force }} {{ static }}
    just compile-h2otils-test {{ type }} {{ force }} {{ threads }}
    just link-h2otils-test {{ type }} {{ static }}

build-htils-test-static force="false" threads=num_cpus():
    just build-htils-test release {{ force }} static {{ threads }}

build-h2otils-test-static force="false" threads=num_cpus():
    just build-h2otils-test release {{ force }} static {{ threads }}

htils-debug static="dynamic" force="false" threads=num_cpus():
    just build-htils debug {{ force }} {{ static }} {{ threads }}

h2otils-debug static="dynamic" force="false" threads=num_cpus():
    just build-h2otils debug {{ force }} {{ static }} {{ threads }}

htils-release static="dynamic" force="false" threads=num_cpus():
    just build-htils release {{ force }} {{ static }}  {{ threads }}

h2otils-release static="dynamic" force="false" threads=num_cpus():
    just build-h2otils release {{ force }} {{ static }}  {{ threads }}

htils-release-static force="false" threads=num_cpus():
    just build-htils-static {{ force }} {{ threads }}

h2otils-release-static force="false" threads=num_cpus():
    just build-h2otils-static {{ force }} {{ threads }}

debug-htils-test force="false" threads=num_cpus():
    just build-htils-test debug {{ force }} {{ threads }}

debug-h2otils-test force="false" threads=num_cpus():
    just build-h2otils-test debug {{ force }} {{ threads }}

release-htils-test force="false" threads=num_cpus():
    just build-htils-test release {{ force }} {{ threads }}

release-h2otils-test force="false" threads=num_cpus():
    just build-h2otils-test release {{ force }} {{ threads }}

clean:
    [[ ! -d {{ lib }} ]] || rm -rf {{ lib }}
    [[ ! -d {{ out }} ]] || rm -rf {{ out }}
    [[ ! -d {{ bin }} ]] || rm -rf {{ bin }}

__bear-compile:
    just compile-htils debug force
    just compile-h2otils debug force
    just compile-htils-test debug force
    just compile-h2otils-test debug force

bear:
    bear -- just __bear-compile
    sed -i 's|"/nix/store/[^"]*gcc[^"]*|\"gcc|g' compile_commands.json
