#!/bin/sh

. $(dirname $0)/tinfra-support/spawn-test-common.sh

test_segv()
{
    local test_name="test_fatal_exception-segv"
    local test_log_file=$(get_test_log_file $test_name)
    report_test_start ${test_name} ./test_fatal_exception segv
    ( 
        cd ${dist}
        ./test_fatal_exception segv 
    ) >> ${test_log_file} 2>&1
    test_exit_code=$?
    if [ ${test_exit_code} != "0" ] ; then
        if grep -q "fatal signal 11 received" ${test_log_file} ; then
            report_result ${test_name} ${test_exit_code} success
        else
            report_result ${test_name} ${test_exit_code} "FAIL, should report exception out stdout/stderr"
            failed=1
        fi        
    else
        report_result ${test_name} ${test_exit_code} "FAIL, should exit with non-zero exitcode"
        failed=1
    fi
}

test_segv
generic_test plain_unittests ./unittests
generic_test valgrind_unittests valgrind --tool=memcheck ./unittests

exit $failed

