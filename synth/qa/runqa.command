#!/bin/bash
# automated build and test script
# can send report to optional email address
# runqa.command  {mailaddr}

QA_MAILADDR=$1

QA_SUMMARY='/tmp/qa_summary.txt'
QA_OUTPUT='/tmp/qa_output.txt'
QA_DETAILS='/tmp/qa_details.txt'
QA_MAKE_OUTPUT='/tmp/qa_make_errors.txt'

if [ "${QA_MAILADDR}" = "" ]
then
    make
    if [ $? != 0 ]
    then
        exit
    fi
else
    make 2> ${QA_MAKE_OUTPUT} > /dev/null
    if [ $? != 0 ]
    then
        mail -s "QA spmidi build FAILED" ${QA_MAILADDR} <${QA_MAKE_OUTPUT}
        exit
    fi
fi

TESTS=`make list_tests`
# TESTS='qa_pitch qa_spmidi qa_memheap'

NUM_PASSED=0
NUM_FAILED=0

# Run all of the tests and keep count of which ones pass and fail.
echo "Test summary `date`" >${QA_SUMMARY}
echo "Test details `date`" >${QA_DETAILS}
for TESTNAME in ${TESTS} "qa_checksum_song ../data/FurryLisa_rt.mid -c1500002998" "qa_checksum_song ../data/ZipKit.mxmf -c4244565239"
do
    echo "Run ${TESTNAME}"
    
    ./${TESTNAME} > ${QA_OUTPUT}
    QA_RESULT=$?
    echo "qa result is " ${QA_RESULT}
    if [ ${QA_RESULT} = 0 ]
    then
        NUM_PASSED=`expr ${NUM_PASSED} + 1`
    else
        NUM_FAILED=`expr ${NUM_FAILED} + 1`
        echo "--------------------------------------------- ${TESTNAME}" >>${QA_DETAILS}
        cat ${QA_OUTPUT} >>${QA_DETAILS}
    fi

    grep "PASSED" ${QA_OUTPUT} >>${QA_SUMMARY}
    grep "FAILED" ${QA_OUTPUT} >>${QA_SUMMARY}
done

# Report results to developer.
if [ ${NUM_FAILED} != 0 ]
then
    SUBJECT="QA Test Failed ERROR ERROR"
else
    SUBJECT="QA Test Passed"
fi

if [ "${QA_MAILADDR}" = "" ]
then
    # put summary at end so it is visible on screen
    cat ${QA_DETAILS}
    cat ${QA_SUMMARY}
    echo ${SUBJECT}
else
    # send summary at top of email
    cat ${QA_MAKE_OUTPUT} >> ${QA_SUMMARY}
    cat ${QA_DETAILS} >> ${QA_SUMMARY}
    mail -s "${SUBJECT}" ${QA_MAILADDR} <${QA_SUMMARY}
fi

exit

