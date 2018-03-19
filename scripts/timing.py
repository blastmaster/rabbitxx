#! /usr/bin/env python3

'''

~/traces/madbench2/scorep-20170613_1523_40520294257341/traces.otf2

dump_events:

process id: 0
filename: /home/soeste/code/MADbench2/files/data
region: fclose
request_size:  0
response size: 0
offset: 0
mode: operation mode: read operation flag: none
kind: close_or_delete
timestamp: 37458393612
Event duration: Enter: 34148338443
Leave: 37459054653
Duration: 3310716210ps

timespan_io output:

total time: 103914us
total file io time: 3849us
total file io metadata time: 4650us
first event time: 4588564
last event time: 37589573617
first event time duration: 4us
last event time duration: 37589us
Clock properties:
ticks per second: 2195022025
start time: 40520194866948
length: 82674186

aggregated I/O transaction time: 11.904ms
'''
def pico2ms(ps):
    return 1.0 * 10e9 * ps

def nano2ms(ps):
    return 1.0 * 10e6 * ps

enter = 34148338443
leave = 37459054653
duration = leave - enter
tps = 2195022025
length = 82674186

print("===From Trace in ticks===")
print("enter: {enter} leave: {leave} duration: {duration}".format(enter=enter, leave=leave, duration=duration))

print("===From Trace in ms===")
print("enter: {enter} leave: {leave} duration: {duration}".format(enter=pico2ms(enter), leave=pico2ms(leave), duration=pico2ms(duration)))


exp_enter = 34.148338
exp_leave = 37.459055
exp_duration_ms = 3.310716

print("===From Vampir in ms===")
print("enter: {enter} leave: {leave} duration: {duration}".format(enter=exp_enter, leave=exp_leave, duration=exp_duration_ms))

print("===Total length===")
print("{}".format(
