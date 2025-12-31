#!/usr/bin/env bash
# This script is designed to be run on a Linux host (i.e., not in xv6)
# You should probably run this script to see what it does first. To do that, you
# may need to make it executable:
#
# chmod +x generate-traces.sh
#
# And then run it from the *root* of your OS directory.
#
#
# For each user space system call stub, this script will print out information
# about its name, arguments, and return values.
#
# You can add information to the stubs that this script will extract. For
# example, instead of:
#
# int write(int, const void *, int);
#
# You can add argument names:
#
# int write(int fd, const void *buf, int count);
#
# And this script will extract them. By default, pointers are assumed to be
# addresses, but if you want the script to treat them like strings you can add a
# bit of metadata with a C comment:
#
# int mkdir(const char* /*str*/ path);
#
# Here's a few examples from a user/user.h file:
# int exit(int status) __attribute__((noreturn));
# int wait(int *status);
# int pipe(int *pipefd);
# int mkdir(const char* /*str*/ path);


# The following code extracts function information from user/user.h based on the
# system calls found in user/usys.pl.

# Retrieves the type of the argument provided as a printf format specifier
get_type() {
    case "${1}" in
    *'/*str*/'*) echo "%s" ;;
    *"*"*) echo "%p" ;;
    *) echo "%ld" ;; # default to int
    esac
}

# Determines which function to call for a given format specifier.
get_conversion() {
    case "${1}" in
        '%p') echo "read_ptr(${2})" ;;
        '%s') echo "read_str(${2})" ;;
        '%ld') echo "read_int(${2})" ;;
    esac
}

# Inspects the user/user.h file to extract system call information
syscall_output=$(
for i in $(sed -n 's/entry("\(.*\)");/\1/p' user/usys.pl); do
    return_value=$(sed -nE \
        "s:\s*([A-Za-z_][A-Za-z0-9_\* ]+)\s+${i}.*:\1:p;"  user/user.h)
    args=$(sed 's/const//g; s/void//g; s/struct//g; s/((noreturn))//g' user/user.h \
        | sed -nE "s:.*\s${i}\((.*)\).*:\1:p")

    # Extract argument names and types for printing
    arg_list=""
    arg_convs=""
    arg_count=0
    export IFS=","
    for arg in $args; do
        arg_name=$(echo "${arg}" | sed -nE 's:.*\b([A-Za-z0-9_]+)\s*:\1:p')
        arg_type=$(get_type "${arg}")
        arg_conv=$(get_conversion "${arg_type}" "${arg_count}")
        arg_list="${arg_list}${arg_name} = ${arg_type}, "
        arg_convs="${arg_convs}${arg_conv}, "
        (( arg_count++ ))
    done

    # Remove extra ', '
    [[ "${arg_count}" -gt 0 ]] && arg_list="${arg_list::-2}"

    # cast the return type, if necessary
    return_type="$(get_type "${return_value}")"
    cast_ret=""
    if [[ "${return_type}" == "%p" ]]; then
        cast_ret="(void *)"
    fi

    # Print out the C code to display system call information. You will probably
    # need to modify this so it can be used to conditionally print out the
    # appropriate info.
    echo "case SYS_${i}:"
    echo "  printf(\"[%d|%s] ${i}(${arg_list}) = $(get_type "${return_value}")\\n\", p->pid, p->name, ${arg_convs} ${cast_ret} ret_val);"
    echo " break;"
done
)

# The information we want has been extracted and stored in $syscall_output.
# Using this information, we'll generate C code below. When we run 'cat <<EOM'
# it tells cat to output everything that follows until the next 'EOM' is found.

cat <<EOM
#include "defs.h"
#include "syscall.h"
#pragma GCC diagnostic ignored "-Wunused-function"

static uint64
read_int(int n)
{
  int i;
  argint(n, &i);
  return i;
}

static void *
read_ptr(int n)
{
  // TODO: read an argument as a pointer. wait() in sysproc.c
  // is a good example of doing this.
  uint64 addr;
  argaddr(n, &addr);
  return (void *)addr;
}

static char *
read_str(int n)
{
  // TODO: read an argument as a string. open() or mkdir() in sysfile.c 
  // are good examples of doing this.
  static char buf[128];
  if (argstr(n, buf, sizeof(buf)) < 0)
  	return "(null)";
  return buf;
}

void
strace(struct proc *p, int syscall_num, uint64 ret_val)
{
  // printf("[strace] syscall: %d\n", syscall_num);
  // This will drop the raw syscall output here, so it's commented out for
  // now... we need to update this function so that it prints system call
  // tracing information (based on the syscall_num passed in).
  switch(syscall_num){
	${syscall_output}
  }
}
EOM
