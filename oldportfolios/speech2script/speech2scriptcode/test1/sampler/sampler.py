import random
import io
import os

#==============================base sentences==============================#
class Script_Sentence(object):
    def __init__(self,subject_list,object_list,verb_list,combined_verb_list,adverb_list,pre_list,mode):
        self.sentence_subject = subject_list
        self.sentence_object = object_list
        self.sentence_verb = verb_list
        self.sentence_combined_verb = combined_verb_list
        self.sentence_adverb = adverb_list
        self.sentence_preposition = pre_list
        self.complete_sentence_so = []
        self.complete_sentence_v = []
        self.mode = mode

#==============================functional sentences==============================#
class Functional_Script_Sentence(Script_Sentence):
    def __init__(self,name_subject,name_object,name_adverb,
                subject_list,object_list,verb_list,combined_verb_list,adverb_list):
        self.script = str(name_subject + "." + name_object + name_adverb + "()")
        super().__init__(subject_list,object_list,verb_list,combined_verb_list,adverb_list)
        

        
class Intvalue_Script_Sentence(Script_Sentence):
    def __init__(self,name_subject,name_object,name_verb,int_value,
                subject_list,object_list,verb_list,combined_verb_list,adverb_list):
        self.script = str(name_subject + "." + name_object + name_verb + "(" + str(int_value) + ")")
        super().__init__(subject_list,object_list,verb_list,combined_verb_list,adverb_list)
        self.int_value = int_value
  
class Sentence_generator(object):
    def __init__(self,target_object):
        self._target_object = target_object
        self.mode = self._target_object.mode
        
    #############################################
    #                   SVO                    #
    #############################################
    #subject-s, verb-v, object-o, combined verb-c, adverb-a
    #subjet and object are must-needed to define device and fuction and one of verb/combined verb with adverb/adverb is must-needed to direct fuction
    #svo
    def svo(self):
        for i in self._target_object.sentence_subject :
            for j in self._target_object.sentence_verb:
                for k in self._target_object.sentence_object:
                    sample_sentence = str(i + " " + j + " " + k)
                    self._target_object.complete_sentence_so.append(sample_sentence)

    #scoa
    def scoa(self):
        for i in self._target_object.sentence_subject :
            for j in self._target_object.sentence_combined_verb:
                for k in self._target_object.sentence_object:
                    for l in self._target_object.sentence_adverb:
                        sample_sentence = str(i + " " + j + " " + k + " " + l)
                        self._target_object.complete_sentence_so.append(sample_sentence)

    #scao
    def scao(self):
        for i in self._target_object.sentence_subject :
            for j in self._target_object.sentence_combined_verb:
                for k in self._target_object.sentence_adverb:
                    for l in self._target_object.sentence_object:
                        sample_sentence = str(i + " " + j + " " + k + " " + l)
                        self._target_object.complete_sentence_so.append(sample_sentence)

    #soa
    def soa(self):
        for i in self._target_object.sentence_subject :
            for j in self._target_object.sentence_object:
                for k in self._target_object.sentence_adverb:
                    sample_sentence = str(i + " " + j + " " + k)
                    self._target_object.complete_sentence_so.append(sample_sentence)
    #osa
    def soa(self):
        for i in self._target_object.sentence_object :
            for j in self._target_object.sentence_preposition:
                for k in self._target_object.sentence_object:
                    for l in self._target_object.sentence_adverb:
                        sample_sentence = str(i + " " + j + " " + k + " " + l)
                        self._target_object.complete_sentence_so.append(sample_sentence)
    
    #vso
    def vos(self):
        for i in self._target_object.sentence_verb:
            for j in self._target_object.sentence_subject:
                for k in self._target_object.sentence_object:
                    sample_sentence = str(i + " " + j + " " + k)
                    self._target_object.complete_sentence_v.append(sample_sentence)
    #vos
    def vos(self):
        for i in self._target_object.sentence_verb:
            for j in self._target_object.sentence_object:
                for k in self._target_object.sentence_preposition:
                    for l in self._target_object.sentence_subject:
                    sample_sentence = str(i + " " + j + " " + k + " " + l)
                    self._target_object.complete_sentence_v.append(sample_sentence)

    #caso
    def caso(self):
        for i in self._target_object.sentence_combined_verb:
            for j in self._target_object.sentence_adverb:
                for k in self._target_object.sentence_subject:
                    for l in self._target_object.sentence_object:
                        sample_sentence = str(i + " " + j + " " + k + " " + l)
                        self._target_object.complete_sentence_v.append(sample_sentence)

    #caos
    def caos(self):
        for i in self._target_object.sentence_combined_verb:
            for j in self._target_object.sentence_adverb:
                for k in self._target_object.sentence_object:
                    for l in self._target_object.sentence_preposition:
                        for m in self._target_object.sentence_subject:
                        sample_sentence = str(i + " " + j + " " + k + " " + l + " " + m)
                        self._target_object.complete_sentence_v.append(sample_sentence)
    #############################################
    #                   SVO                     #
    #############################################

    #############################################
    #                   S V                     #
    #############################################
    #sv
    def sv(self):
        for i in self._target_object.sentence_subject:
            for j in self._target_object.sentence_verb:
                sample_sentence = str(i + " " + j)
                self._target_object.complete_sentence_so.append(sample_sentence)
    
    #sca
    def sca(self):
        for i in self._target_object.sentence_subject:
            for j in self._target_object.sentence_combined_verb:
                for k in self._target_object.sentence_adverb:
                    sample_sentence = str(i + " " + j + " " + k)
                    self._target_object.complete_sentence_so.append(sample_sentence)
    
    #sa
    def sa(self):
        for i in self._target_object.sentence_subject:
            for j in self._target_object.sentence_adverb:
                sample_sentence = str(i + " " + j)
                self._target_object.complete_sentence_so.append(sample_sentence)
                
    #vs
    def vs(self):
        for i in self._target_object.sentence_verb:
            for j in self._target_object.sentence_subject:
                sample_sentence = str(i + " " + j)
                self._target_object.complete_sentence_v.append(sample_sentence)
                
    #csa
    def csa(self):
        for i in self._target_object.sentence_combined_verb:
            for j in self._target_object.sentence_subject:
                for k in self._target_object.sentence_adverb:
                    sample_sentence = str(i + " " + j + " " + k)
                    self._target_object.complete_sentence_v.append(sample_sentence)

    #cas
    def cas(self):
        for i in self._target_object.sentence_combined_verb:
            for j in self._target_object.sentence_adverb:
                for k in self._target_object.sentence_subject:
                    sample_sentence = str(i + " " + j + " " + k)
                    self._target_object.complete_sentence_v.append(sample_sentence)
    #############################################
    #                   S V                     #
    #############################################
    
    #############################################
    #                   INT                     #
    #############################################
    #i stands for int
    #svoi
    def svoi(self):
        for i in self._target_object.sentence_subject :
            for j in self._target_object.sentence_verb:
                for k in self._target_object.sentence_object:
                    sample_sentence = str(i + " " + j + " " + k + " " + str(self._target_object.int_value))
                    self._target_object.complete_sentence_so.append(sample_sentence)
    #sovi
    def sovi(self):
        for i in self._target_object.sentence_subject :
            for j in self._target_object.sentence_object:
                for k in self._target_object.sentence_verb:
                    sample_sentence = str(i + " " + j + " " + k + " " + str(self._target_object.int_value))
                    self._target_object.complete_sentence_so.append(sample_sentence)

    #scoai
    def scoai(self):
        for i in self._target_object.sentence_subject :
            for j in self._target_object.sentence_combined_verb:
                for k in self._target_object.sentence_object:
                    for l in self._target_object.sentence_adverb:
                        sample_sentence = str(i + " " + j + " " + k + " " + l + " to " + str(self._target_object.int_value))
                        self._target_object.complete_sentence_so.append(sample_sentence)

    #scaoi
    def scaoi(self):
        for i in self._target_object.sentence_subject :
            for j in self._target_object.sentence_combined_verb:
                for k in self._target_object.sentence_adverb:
                    for l in self._target_object.sentence_object:
                        sample_sentence = str(i + " " + j + " " + k + " " + l + " to " + str(self._target_object.int_value))
                        self._target_object.complete_sentence_so.append(sample_sentence)

    #socai
    def socai(self):
        for i in self._target_object.sentence_subject :
            for j in self._target_object.sentence_object:
                for k in self._target_object.sentence_combined_verb:
                    for l in self._target_object.sentence_adverb:
                        sample_sentence = str(i + " " + j + " " + k + " " + l + " to " + str(self._target_object.int_value))
                        self._target_object.complete_sentence_so.append(sample_sentence)

    #soai
    def soai(self):
        for i in self._target_object.sentence_subject :
            for j in self._target_object.sentence_object:
                for k in self._target_object.sentence_adverb:
                    sample_sentence = str(i + " " + j + " " + k + " to " + str(self._target_object.int_value))
                    self._target_object.complete_sentence_so.append(sample_sentence)
    #soi
    def soi(self):
        for i in self._target_object.sentence_subject :
            for j in self._target_object.sentence_object:
                sample_sentence = str(i + " " + j + " to " + str(self._target_object.int_value))
                self._target_object.complete_sentence_so.append(sample_sentence)
                    
    #vsoi
    def vsoi(self):
        for i in self._target_object.sentence_verb:
            for j in self._target_object.sentence_subject:
                for k in self._target_object.sentence_object:
                    sample_sentence = str(i + " " + j + " " + k + " to " + str(self._target_object.int_value))
                    self._target_object.complete_sentence_v.append(sample_sentence)

    #vois
    def vois(self):
        for i in self._target_object.sentence_verb:
            for j in self._target_object.sentence_object:
                for k in self._target_object.sentence_subject:
                    sample_sentence = str(i + " " + j +  " to " + str(self._target_object.int_value) + " of " + k)
                    self._target_object.complete_sentence_v.append(sample_sentence)

    #csoai
    def csoai(self):
        for i in self._target_object.sentence_combined_verb:
            for j in self._target_object.sentence_subject:
                for k in self._target_object.sentence_object:
                    for l in self._target_object.sentence_adverb:
                        sample_sentence = str(i + " " + j + " " + k + " " + l + " to " + str(self._target_object.int_value))
                        self._target_object.complete_sentence_v.append(sample_sentence)

    #cosai
    def cosai(self):
        for i in self._target_object.sentence_combined_verb:
            for j in self._target_object.sentence_object:
                for k in self._target_object.sentence_subject:
                    for l in self._target_object.sentence_adverb:
                        sample_sentence = str(i + " " + j + " of " + k + " " + l + " to " + str(self._target_object.int_value))
                        self._target_object.complete_sentence_v.append(sample_sentence)

    #casoi
    def casoi(self):
        for i in self._target_object.sentence_combined_verb:
            for j in self._target_object.sentence_adverb:
                for k in self._target_object.sentence_subject:
                    for l in self._target_object.sentence_object:
                        sample_sentence = str(i + " " + j + " " + k + " " + l + " to " + str(self._target_object.int_value))

    #caosi
    def caosi(self):
        for i in self._target_object.sentence_combined_verb:
            for j in self._target_object.sentence_adverb:
                for k in self._target_object.sentence_object:
                    for l in self._target_object.sentence_subject:
                        sample_sentence = str(i + " " + j + " " + k + " of " + l + " to " + str(self._target_object.int_value))
                        self._target_object.complete_sentence_v.append(sample_sentence)

    #osvi
    def osvi(self):
        for i in self._target_object.sentence_object:
            for j in self._target_object.sentence_subject:
                for k in self._target_object.sentence_verb:
                    sample_sentence = str(i + " of " + j + " " + k + " to " + str(self._target_object.int_value))
                    self._target_object.complete_sentence_so.append(sample_sentence)
    #oscai
    def oscai(self):
        for i in self._target_object.sentence_object:
            for j in self._target_object.sentence_subject:
                for k in self._target_object.sentence_combined_verb:
                    for l in self._target_object.sentence_adverb:
                        sample_sentence = str(i + " of " + j + " " + k + " " + l + " to " + str(self._target_object.int_value))
                        self._target_object.complete_sentence_so.append(sample_sentence)

    #osai
    def osai(self):
        for i in self._target_object.sentence_object:
            for j in self._target_object.sentence_subject:
                for k in self._target_object.sentence_adverb:
                    sample_sentence = str(i + " of " + j + " " + k + " to " + str(self._target_object.int_value))
                    self._target_object.complete_sentence_so.append(sample_sentence)
    
    #############################################
    #                   INT                     #
    #############################################
    
                    
    def make_all(self):
        if self.mode == "SVO":
            self.svo()
            self.scoa()
            self.scao()
            self.soa()
            self.osa()
            self.vso()
            self.vos()
            self.caso()
            self.caos()
        elif self.mode == "SV":
            self.sv()
            self.sca()
            self.sa()
            self.vs()
            self.csa()
            self.cas()
        elif self.mode == "int":
            self.svoi()
            self.sovi()
            self.scoai()
            self.scaoi()
            self.socai()
            self.soai()
            self.soi()
            self.vsoi()
            self.vois()
            self.csoai()
            self.cosai()
            self.casoi()
            self.caosi()
            self.osvi()
            self.oscai()
            self.osai()
#==============================functional sentences==============================#

#==============================trigger sentences==============================#
#trigger is "if","wait until"
class operand_container(object):
    def __init__(self,script,parsing_list,additional_operator_equal_smaller=[],additional_operator_equal_bigger=[],
                    additional_operator_smaller=[],additional_operator_bigger=[]):
        self.script = script
        self.parsing_list = parsing_list
        self.additional_operator_equal_smaller = additional_operator_equal_smaller
        self.additional_operator_equal_bigger = additional_operator_equal_bigger
        self.additional_operator_smaller = additional_operator_smaller
        self.additional_operator_bigger = additional_operator_bigger

class Trigger_Script_Sentence(object):
    def __init__(self, conjuction, operand_container1, operator_12, operand_container2,
                operand_container3, operator_34, operand_container4):
        
        self.complete_phrase = []
        self.operand1_list = operand_container1.parsing_list
        self.operand2_list = operand_container2.parsing_list
        self.operator_12 = []
        if (operator_12 == "=="):
            self.operator12_list = ["same with", "exactly same with", "no different from", "equal with"]
        elif (operator_12 == "!="):
            self.operator12_list = ["not same with", "different from", "not equal with"]
        elif (operator_12 == "<="):
            self.operator12_list = ["equal or smaller than", "equal or less than", "equal or under",
                                    "no greater than", "no more than", "not greater than", "not more than"]
            self.operator12_list = self.operator12_list + operand_container1.additional_operator_equal_smaller +                                                      operand_container2.additional_operator_equal_smaller
        elif (operator_12 == ">="):
            self.operator12_list = ["equal or greater than", "equal or more than", "equal or over",
                                    "no smaller than", "no less than", "not smaller than", "not less than"]
            self.operator12_list = self.operator12_list + operand_container1.additional_operator_equal_bigger +                                                      operand_container2.additional_operator_equal_bigger
        elif (operator_12 == "<"):
            self.operator12_list = ["smaller than", "less than", "under"]
            self.operator12_list = self.operator12_list + operand_container1.additional_operator_smaller +                                                      operand_container2.additional_operator_smaller
        elif (operator_12 == ">"):
            self.operator12_list = ["greater than", "more than", "over"]
            self.operator12_list = self.operator12_list + operand_container1.additional_operator_bigger +                                                      operand_container2.additional_operator_bigger
        
        if (conjuction is not None):
            self.conjuction = conjuction
            self.operand3_list = operand_container3.parsing_list
            self.operand4_list = operand_container4.parsing_list
            self.operator_34 = []
            if (operator_34 == "=="):
                self.operator34_list = ["same with", "exactly same with", "no different from", "equal with"]
            elif (operator_34 == "!="):
                self.operator34_list = ["not same with", "different from", "not equal with"]
            elif (operator_34 == "<="):
                self.operator34_list = ["equal or smaller than", "equal or less than", "equal or under",
                                        "no greater than", "no more than", "not greater than", "not more than"]
                self.operator34_list = self.operator34_list + operand_container3.additional_operator_equal_smaller +                                                     operand_container4.additional_operator_equal_smaller
            elif (operator_34 == ">="):
                self.operator34_list = ["equal or greater than", "equal or more than", "equal or over",
                                        "no smaller than", "no less than", "not smaller than", "not less than"]
                self.operator34_list = self.operator34_list + operand_container3.additional_operator_equal_bigger +                                                      operand_container4.additional_operator_equal_bigger
            elif (operator_34 == "<"):
                self.operator34_list = ["smaller than", "less than", "under"]
                self.operator34_list = self.operator34_list + operand_container3.additional_operator_smaller +                                                      operand_container4.additional_operator_smaller
            elif (operator_34 == ">"):
                self.operator34_list = ["greater than", "more than", "over"]
                self.operator34_list = self.operator34_list + operand_container3.additional_operator_bigger +                                                      operand_container4.additional_operator_bigger
        
        
    
class Conditional_Script_Sentence(Trigger_Script_Sentence):
    def __init__(self, trigger,
                operand_container1, operator_12, operand_container2,
                conjuction, operand_container3, operator_34, operand_container4):
        self.script = str(trigger + " (" + operand_container1.script + " " + operator_12 + " " + operand_container2.script)
        if (conjuction is not None):
            self.script = self.script + " " + conjuction + " " + operand_container3.script + " " + operator_34 + " " + operand_container4.script
        self.script = self.script + ")"
        if (trigger == "if"):
            self.trigger = "if"
            self.trigger_list = ["if", "on condition that"]
        elif (trigger == "wait until"):
            self.trigger = "wait until"
            self.trigger_list = ["wait until", "hold until", "stop until", "stay until"]
        super().__init__(conjuction, operand_container1, operator_12, operand_container2,
                                    operand_container3, operator_34, operand_container4)



class condition1_generator(object):
    def __init__(self,target_object):
        self._target_object = target_object
        for trigger in self._target_object.trigger_list:
            for operand1 in self._target_object.operand1_list:
                for operator1 in self._target_object.operator12_list:
                    for operand2 in self._target_object.operand2_list:
                        sample_sentence = str(trigger + " " + operand1 + " is " + operator1 + " " + operand2 + ",")
                        self._target_object.complete_phrase.append(sample_sentence)
                

class condition2_generator(object):
    def __init__(self,target_object):
        self._target_object = target_object
        for trigger in self._target_object.trigger_list:
            for operand1 in self._target_object.operand1_list:
                for operator1 in self._target_object.operator12_list:
                    for operand2 in self._target_object.operand2_list:
                        for operand3 in self._target_object.operand3_list:
                            for operator2 in self._target_object.operator34_list:
                                for operand4 in self._target_object.operand4_list:
                                    sample_sentence = str(trigger + " " + operand1+ " is " + operator1 + " " + operand2 + " " +
                                    self._target_object.conjuction + " " +
                                    operand3 + " is " + operator2 + " " + operand4 + "," )
                                    self._target_object.complete_phrase.append(sample_sentence)
#==============================trigger sentences==============================#

#==============================loop sentences==============================#
#loop is "loop"
class Loop_Script_Sentence(Trigger_Script_Sentence):
    def __init__(self, cycle_number, cycle_time, operand_container1, operator_12, operand_container2, conjuction,
                operand_container3, operator_34, operand_container4):
        self.script = str("loop" + " (" + str(cycle_number)  + " " + cycle_time)
#        +", " + operand_container1.script + " " +operator_12 + " " + operand_container2.script)
#        if (conjuction is not None):
#            self.script = self.script + " " + conjuction + " " + operand_container3.script + " " + operator_34 + " " + operand_container4.script
            
        self.script = self.script + ")"
        super().__init__(conjuction, operand_container1, operator_12, operand_container2,
                                    operand_container3, operator_34, operand_container4)
        self.frequency = " "
        if cycle_time == "SEC":
            if cycle_number == 1:
                self.frequency = "every second"
            else:
                self.frequency = "every " + str(cycle_number) + " seconds"
        elif cycle_time == "MIN":
            if cycle_number == 1:
                self.frequency = "every minute"
            else:
                self.frequency = "every " + str(cycle_number) + " minutes"
        elif cycle_time == "HOUR":
            if cycle_number == 1:
                self.frequency = "every hour"
            else:
                self.frequency = "every " + str(cycle_number) + " hours"
        elif cycle_time == "DAY":
            if cycle_number == 1:
                self.frequency = "every day"
            else:
                self.frequency = "every " + str(cycle_number) + " days"
       
        self.trigger_list = ["once in " + self.frequency, self.frequency]


class loop1_generator(object):
    def __init__(self,target_object):
        self._target_object = target_object
        for trigger in self._target_object.trigger_list:
#            for operand1 in self._target_object.operand1_list:
#                for operator1 in self._target_object.operator12_list:
#                    for operand2 in self._target_object.operand2_list:
            sample_sentence = str(trigger)
#                         + operand1 + " is " + operator1 + " " + operand2)
            self._target_object.complete_phrase.append(sample_sentence)
                

#class loop2_generator(object):
#    def __init__(self,target_object):
#        self._target_object = target_object
#        for trigger in self._target_object.trigger_list:
#            for operand1 in self._target_object.operand1_list:
#                for operator1 in self._target_object.operator12_list:
#                    for operand2 in self._target_object.operand2_list:
#                        for operand3 in self._target_object.operand3_list:
#                            for operator2 in self._target_object.operator34_list:
#                                for operand4 in self._target_object.operand4_list:
#                                    sample_sentence = str(trigger + " " + operand1+ " is " + operator1 + " " + operand2 + " " +
#                                    self._target_object.conjuction + " " +
#                                    operand3 + " is " + operator2 + " " + operand4)
#                                    self._target_object.complete_phrase.append(sample_sentence)
#==============================loop sentences==============================#



#start of program
if __name__ == "__main__":
    
    funtion_list = []
    
#==============================functional sentences==============================#
    
    #----------------------------TV-----------------------------------#
    #TV.turnon()
    TVturnon = Functional_Script_Sentence("TV","turn","on",["TV","television"],
                    [],
                    ["start","show", "power on", "wake", "wake up"],
                    ["turn"],["on"],[],)
    funtion_list.append(TVturnon)
    
    #TV.turnoff()
    TVturnoff = Functional_Script_Sentence("TV","turn","off",["TV","television"],
                    [],
                    ["end","quit", "finish", "power off", "sleep"],
                    ["turn"],["off"])
    funtion_list.append(TVturnoff)
    
    #TV.volumeup()
    TVvolumeup = Functional_Script_Sentence("TV","volume","up",["TV","television"],
                    ["volume","sound","audio","voice","tone"],
                    ["raise","increase","louden","more"],
                    ["turn","change"],["up","louder"])
    funtion_list.append(TVvolumeup)
    
    #TV.volumedown()
    TVvolumedown = Functional_Script_Sentence("TV","volume","down",["TV","television"],
                    ["volume","sound","audio","voice","tone"],
                    ["reduce","decrease","quieten","drop"],
                    ["turn","change"],["down", "quieter"])
    funtion_list.append(TVvolumedown)
    
    #TV.channelup()
    TVchannelup = Functional_Script_Sentence("TV","channel","up",["TV","television"],
                    ["channel"],
                    ["raise","increase"],
                    ["change"],["up","higher"])
    funtion_list.append(TVchannelup)
    
    #TV.channeldown()
    TVchanneldown = Functional_Script_Sentence("TV","channel","down",["TV","television"],
                    ["channel"],
                    ["reduce","decrease","lower"],
                    ["change"],["down"], ["of"])
    funtion_list.append(TVchanneldown)
    
    #TV.channelchange(int)
    for i in range(99):
        temp_num = i
        channelchange = Intvalue_Script_Sentence("TV","channel","change",temp_num,["TV","television"],
                        ["channel"],
                        ["watch"],
                        ["go", "change", "tune", "switch", "move"],["to", "into"], ["of"])
        funtion_list.append(channelchange)
    
    #----------------------------white led light-----------------------------------#
    #whiteledlight.turnon()
    whiteledlightturnon = Functional_Script_Sentence("whiteledlight","turn","on",
                    ["white light","white led", "main light", "led lamp"],
                    [],
                    ["start", "power on", "wake", "wake up"],
                    ["turn"],["on"])
    funtion_list.append(whiteledlightturnon)
    
    #whiteledlight.turnoff()
    whiteledlightturnoff = Functional_Script_Sentence("whiteledlight","turn","off",
                    ["white light","white led", "main light", "led lamp"],
                    [],
                    ["end", "finish", "power off", "sleep"],
                    ["turn"],["off"])
    funtion_list.append(whiteledlightturnoff)
    
    #whiteledlight.brightnessup()
    whiteledlightbrightnessup = Functional_Script_Sentence("whiteledlight","brightness","up",
                    ["white light","white led", "main light", "led lamp"],
                    ["brightness","lumen"],
                    ["up","raise","increase","brighten","heighten"],
                    [],["brighter", "higher"],mode = "dual")
    funtion_list.append(whiteledlightbrightnessup)
        
    #whiteledlight.brightnessdown()
    whiteledlightbrightnessdown = Functional_Script_Sentence("whiteledlight","brightness","down",
                    ["white light","white led", "main light", "led lamp"],
                    ["brightness","lumen"],
                    ["down","reduce","decrease","darken","dim","lower"],
                    [],["darker", "dimmer"],mode = "dual")
    funtion_list.append(whiteledlightbrightnessdown)
    
    #----------------------------red led light-----------------------------------#
    #redledlight.turnon()
    redledlightturnon = Functional_Script_Sentence("redledlight","turn","on",["red light","red led"],
                    [],
                    ["start", "power on", "wake", "wake up"],
                    ["turn"],["on"])
    funtion_list.append(redledlightturnon)
    
    #redledlight.turnoff()
    redledlightturnoff = Functional_Script_Sentence("redledlight","turn","off",["red light","red led"],
                    [],
                    ["end", "finish", "power off", "sleep"],
                    ["turn"],["off"])
    funtion_list.append(redledlightturnoff)
    
    #redledlight.brightnessup()
    redledlightbrightnessup = Functional_Script_Sentence("redledlight","brightness","up",["red light","red led"],
                    ["brightness","lumen"],
                    ["up","raise","increase","brighten","heighten"],
                    [],["brighter", "higher"],mode = "dual")
    funtion_list.append(redledlightbrightnessup)
    
    #redledlight.brightnessdown()
    redledlightbrightnessdown = Functional_Script_Sentence("redledlight","brightness","down",["red light","red led"],
                    ["brightness","lumen"],
                    ["down","reduce","decrease","darken","dim","lower"],
                    [],["darker", "dimmer"],mode = "dual")
    funtion_list.append(redledlightbrightnessdown)
        
    #----------------------------blue led light-----------------------------------#
    #blueledlight.turnon()
    blueledlightturnon = Functional_Script_Sentence("blueledlight","turn","on",["blue light","blue led"],
                    [],
                    ["start", "power on", "wake", "wake up"],
                    ["turn"],["on"])
    funtion_list.append(blueledlightturnon)
    
    #blueledlight.turnoff()
    blueledlightturnoff = Functional_Script_Sentence("blueledlight","turn","off",["blue light","blue led"],
                    [],
                    ["end", "finish", "power off", "sleep"],
                    ["turn"],["off"])
    funtion_list.append(blueledlightturnoff)
    
    #blueledlight.brightnessup()
    blueledlightbrightnessup = Functional_Script_Sentence("blueledlight","brightness","up",["blue light","blue led"],
                    ["brightness","lumen"],
                    ["up","raise","increase","brighten","heighten"],
                    [],["brighter", "higher"],mode = "dual")
    funtion_list.append(blueledlightbrightnessup)
        
    #blueledlight.brightnessdown()
    blueledlightbrightnessdown = Functional_Script_Sentence("blueledlight","brightness","down",["blue light","blue led"],
                    ["brightness","lumen"],
                    ["down","reduce","decrease","darken","dim","lower"],
                    [],["darker", "dimmer"],mode = "dual")
    funtion_list.append(blueledlightbrightnessdown)
    
    #----------------------------green led light-----------------------------------#
    #greenledlight.turnon()
    greenledlightturnon = Functional_Script_Sentence("greenledlight","turn","on",["green light","green led"],
                    [],
                    ["start", "power on", "wake", "wake up"],
                    ["turn"],["on"])
    funtion_list.append(greenledlightturnon)
    
    #greenledlight.turnoff()
    greenledlightturnoff = Functional_Script_Sentence("greenledlight","turn","off",["green light","green led"],
                    [],
                    ["end", "finish", "power off", "sleep"],
                    ["turn"],["off"])
    funtion_list.append(greenledlightturnoff)
    
    #greenledlight.brightnessup()
    greenledlightbrightnessup = Functional_Script_Sentence("greenledlight","brightness","up",["green light","green led"],
                    ["brightness","lumen"],
                    ["up","raise","increase","brighten","heighten"],
                    [],["brighter", "higher"], mode = "dual")
    funtion_list.append(greenledlightbrightnessup)
    
    #greenledlight.brightnessdown()
    greenledlightbrightnessdown = Functional_Script_Sentence("greenledlight","brightness","down",["green light","green led"],
                    ["brightness","lumen"],
                    ["down","reduce","decrease","darken","dim","lower"],
                    [],["darker", "dimmer"], mode = "dual")
    funtion_list.append(greenledlightbrightnessdown)
        
    #----------------------------super sonic sensor-----------------------------------#
    #supersonicsensor.detectobject()
    supersonicsensordetectobject = Functional_Script_Sentence("supersonicsensor","detect","object",["super sonic sensor","distance sensor", "sonar"],
                    [],
                    ["detect object","detect something","detect","find object","find something","find obstacle","encounter object","encounter something","encounter obstacle"],
                    [],[])
    funtion_list.append(supersonicsensordetectobject)
    
    #supersonicsensor.checkdistance()
    supersonicsensorcheckdistance = Functional_Script_Sentence("supersonicsensor","check","distance",
                    ["super sonic sensor","distance sensor", "sonar"],
                    [],
                    ["detect distance","detect space","find distance","find space","check distance","check space"],
                    [],[])
    funtion_list.append(supersonicsensorcheckdistance)
    
    #----------------------------microphone-----------------------------------#
    #microphone.turnon()
    microphoneturnon = Functional_Script_Sentence("microphone","turn","on",["microphone","mike"],
                    [],
                    ["start", "power on", "wake", "wake up"],
                    ["turn"],["on"])
    funtion_list.append(microphoneturnon)
    
    #microphone.turnoff()
    microphoneturnoff = Functional_Script_Sentence("microphone","turn","off",["microphone","mike"],
                    [],
                    ["end", "finish", "power off", "sleep"],
                    ["turn"],["off"])
    funtion_list.append(microphoneturnoff)
    
    #microphone.recordstart()
    microphonerecordstart = Functional_Script_Sentence("microphone","record","start",["microphone","mike"],
                    ["record","recording","copying"],
                    ["start", "begin"],
                    [],["on"])
    funtion_list.append(microphonerecordstart)
    
    #microphone.recordfinish()
    microphonerecordfinish = Functional_Script_Sentence("microphone","record","finish",["microphone","mike"],
                    ["record","recording","copying"],
                    ["end", "finish"],
                    [],["off"])
    funtion_list.append(microphonerecordfinish)
    
    #microphone.volumeup()
    microphonevolumeup = Functional_Script_Sentence("microphone","volume","up",["microphone","mike"],
                    ["volume","sound","audio","voice","tone"],
                    ["raise","increase","louden","heighten","more"],
                    ["turn","change"],["up","louder","higher"])
    funtion_list.append(microphonevolumeup)
    
    #microphone.volumedown()
    microphonevolumedown = Functional_Script_Sentence("microphone","volume","down",["microphone","mike"],
                    ["volume","sound","audio","voice","tone"],
                    ["reduce","decrease","quiet","lower", "drop"],
                    ["turn","change"],["down","quieter"])
    funtion_list.append(microphonevolumedown)
    
    #----------------------------speaker-----------------------------------#
    #speaker.turnon()
    speakerturnon = Functional_Script_Sentence("speaker","turn","on",["speaker","loud speaker", "speaker system", "stereo", "stereo speaker"],
                    [],
                    ["start", "power on", "wake", "wake up"],
                    ["turn"],["on"])
    funtion_list.append(speakerturnon)
    
    #speaker.turnoff()
    speakerturnoff = Functional_Script_Sentence("speaker","turn","off",["speaker","loud speaker", "speaker system", "stereo", "stereo speaker"],
                    [],
                    ["end", "finish", "power off", "sleep"],
                    ["turn"],["off"])
    funtion_list.append(speakerturnoff)
    
    #speaker.playstart()
    speakerplaystart = Functional_Script_Sentence("speaker","play","start",["speaker","loud speaker", "speaker system", "stereo", "stereo speaker"],
                    ["play","playing"],
                    ["start", "begin"],
                    [],["on"])
    funtion_list.append(speakerplaystart)
    
    #speaker.playfinish()
    speakerplayfinish = Functional_Script_Sentence("speaker","play","finish",["speaker","loud speaker", "speaker system", "stereo", "stereo speaker"],
                    ["play","playing"],
                    ["end", "finish"],
                    [],["off"])
    funtion_list.append(speakerplayfinish)
    
    #speaker.volumeup()
    speakervolumeup = Functional_Script_Sentence("speaker","volume","up",["speaker","loud speaker", "speaker system", "stereo", "stereo speaker"],
                    ["volume","sound","audio","voice","tone"],
                    ["raise","increase","louden","heighten","more"],
                    ["turn","change"],["up","louder"])
    funtion_list.append(speakervolumeup)
    
    #speaker.volumedown()
    speakervolumedown = Functional_Script_Sentence("speaker","volume","down",["speaker","loud speaker", "speaker system", "stereo", "stereo speaker"],
                    ["volume","sound","audio","voice","tone"],
                    ["reduce","decrease","quiet","lower", "drop"],
                    ["turn","change"],["down","quieter"])
    funtion_list.append(speakervolumedown)
    
    #----------------------------door-----------------------------------#
    #door.open()
    dooropen = Functional_Script_Sentence("door","open","",["door","gate"],
                    [],
                    ["open"],
                    [],[])
    funtion_list.append(dooropen)
    
    #door.close()
    doorclose = Functional_Script_Sentence("door","close","",["door","gate"],
                    [],
                    ["close","shut"],
                    [],[])
    funtion_list.append(doorclose)
    
    #door.lock()
    doorlock = Functional_Script_Sentence("door","lock","",["door","gate"],
                    [],
                    ["lock","secure"],
                    [],[])
    funtion_list.append(doorlock)
    
    #door.unlock()
    doorunlock = Functional_Script_Sentence("door","unlock","",["door","gate"],
                    [],
                    ["unlock"],
                    [],[])
    funtion_list.append(doorunlock)
    
    #----------------------------robot-----------------------------------#
    #robot.turnon()
    robotturnon = Functional_Script_Sentence("robot","turn","on",["robot"],
                    [],
                    ["start", "power on", "wake", "wake up"],
                    ["turn"],["on"])
    funtion_list.append(robotturnon)
    
    #robot.turnoff()
    robotturnoff = Functional_Script_Sentence("robot","turn","off",["robot"],
                    [],
                    ["end", "finish", "power off", "sleep"],
                    ["turn"],["off"])
    funtion_list.append(robotturnoff)
    
    #robot.movefoward()
    robotmovefoward = Functional_Script_Sentence("robot","move","forward",["robot"],
                    [],
                    ["move forward", "move stright", "go forward", "go stright"],
                    [],[])
    funtion_list.append(robotmovefoward)
     
    #robot.movebackward()
    robotmovebackward = Functional_Script_Sentence("robot","move","backward",["robot"],
                    [],
                    ["move backward", "move back", "go backward", "go back"],
                    [],[])
    funtion_list.append(robotmovebackward)
    
    #robot.moveright()
    robotmoveright = Functional_Script_Sentence("robot","move","right",["robot"],
                    [],
                    ["move right", "move right side", "go right", "go right side"],
                    [],[])
    funtion_list.append(robotmoveright)
    
    #robot.moveleft()
    robotmoveleft = Functional_Script_Sentence("robot","move","left",["robot"],
                    [],
                    ["move left", "move left side", "go left", "go left side"],
                    [],[])
    funtion_list.append(robotmoveleft)

    #----------------------------camera-----------------------------------#
    #camera.turnon()
    cameraturnon = Functional_Script_Sentence("camera","turn","on",["camera", "cam", "CCTV", "vision sensor"],
                    [],
                    ["start", "power on", "wake", "wake up"],
                    ["turn"],["on"])
    funtion_list.append(cameraturnon)
    
    #camera.turnoff()
    cameraturnoff = Functional_Script_Sentence("camera","turn","off",["camera", "cam", "CCTV", "vision sensor"],
                    [],
                    ["start", "power off", "sleep"],
                    ["turn"],["off"])
    funtion_list.append(cameraturnoff)
    
    #camera.takepicture()
    cameratakepicture = Functional_Script_Sentence("camera","take","picture",["camera", "cam", "CCTV", "vision sensor"],
                    ["picture", "shot", "photo","snapshot","image"],
                    ["take", "get", "shoot"],
                    [],[])
    funtion_list.append(cameratakepicture)
    
    #camera.detectpeople()
    cameradetectpeople = Functional_Script_Sentence("camera","detect","people",["camera", "cam", "CCTV", "vision sensor"],
                    [],
                    ["detect people", "detect person", "detect man", "detect men", "detect human",
                    "distinguish people", "distinguish person", "distinguish man", "distinguish men", "distinguish human",
                    "find people", "find person", "find man", "find men", "find human"],
                    [],[])
    funtion_list.append(cameradetectpeople)
    
    #camera.detectdog()
    cameradetectdog = Functional_Script_Sentence("camera","detect","dog",["camera", "cam", "CCTV", "vision sensor"],
                    [],
                    ["detect dog", "detect hound", "detect puppy", "detect doggy", "detect pup",
                    "distinguish dog", "distinguish hound", "distinguish puppy", "distinguish doggy", "distinguish pup",
                    "find dog", "find hound", "find puppy", "find doggy", "find pup",],
                    [],[])
    funtion_list.append(cameradetectdog)
    
    #camera.detectsign()
    cameradetectsign = Functional_Script_Sentence("camera","detect","sign",["camera", "cam", "CCTV", "vision sensor"],
                    [],
                    ["detect sign", "detect mark", "detect beacon", "detect symbol",
                    "distinguish sign", "distinguish mark", "distinguish beacon", "distinguish symbol",
                    "find sign", "find mark", "find beacon", "find symbol"],
                    [],[])
    funtion_list.append(cameradetectsign)

    #----------------------------airconditioner-----------------------------------#
    #airconditioner.turnon()
    airconditionerturnon = Functional_Script_Sentence("airconditioner","turn","on",["air conditioner", "air-con", "air cooler", "air cooling device"],
               [],
               ["start", "power on", "wake", "wake up"],
               ["turn"],["on"])
    funtion_list.append(airconditionerturnon)

    #airconditioner.turnoff()
    airconditionerturnoff = Functional_Script_Sentence("airconditioner","turn","off",["air conditioner", "air-con", "air cooler", "air cooling device"],
               [],
               ["end", "finish", "power off", "sleep"],
               ["turn"],["off"])
    funtion_list.append(airconditionerturnoff)

    #airconditioner.temperatureup()
    airconditionertemperatureup = Functional_Script_Sentence("airconditioner","temperature","up", ["air conditioner", "air-con", "air cooler", "air cooling device"],
               ["temperature", "target temperature"],
               ["raise","increase","make hotter","make warmer"],
               ["change"],["up", "hotter", "warmer"])
    funtion_list.append(airconditionertemperatureup)

    #airconditioner.temperaturedown()
    airconditionertemperaturedown = Functional_Script_Sentence("airconditioner","temperature","down",["air conditioner", "air-con", "air cooler", "air cooling device"],
               ["temperature", "target temperature", "air temperature"],
               ["reduce","decrease","make colder","make cooler","lower", "drop"],
               ["change"],["down", "colder", "cooler"])
    funtion_list.append(airconditionertemperaturedown)
    #----------------------------heater-----------------------------------#
    #heater.turnon()
    heaterturnon = Functional_Script_Sentence("heater","turn","on",["heater", "boiler", "warmer", "heating device", "heating system", "radiator"],
                  [],
                  ["start", "power on", "wake", "wake up"],
                  ["turn"],["on"])
    funtion_list.append(heaterturnon)

    #heater.turnoff()
    heaterturnoff = Functional_Script_Sentence("heater","turn","off",["heater", "boiler", "warmer", "heating device", "heating system", "radiator"],
                  [],
                  ["end", "finish", "power off", "sleep"],
                  ["turn"],["off"])
    funtion_list.append(heaterturnoff)

    #heater.temperatureup()
    heatertemperatureup = Functional_Script_Sentence("heater","temperature","up", ["heater", "boiler", "warmer", "heating device", "heating system", "radiator"],
                  ["temperature", "heat","target temperature"],
                  ["raise","increase"],
                  ["change"],["up", "hotter", "warmer"])
    funtion_list.append(heatertemperatureup)

    #heater.temperaturedown()
    heatertemperaturedown = Functional_Script_Sentence("heater","temperature","down",["heater", "boiler", "warmer", "heating device", "heating system", "radiator"],
                  ["temperature","target temperature", "heat","target temperature"],
                  ["reduce","decrease","lower", "drop"],
                  ["change"],["down", "colder", "cooler"])
    funtion_list.append(heatertemperaturedown)
       
    #----------------------------humidifier-----------------------------------#
    #humidifier.turnon()
    humidifierturnon = Functional_Script_Sentence("humidifier","turn","on",["humidifier", "moisturizer", "humidification device"],
                  [],
                  ["start", "power on", "wake", "wake up"],
                  ["turn"],["on"])
    funtion_list.append(humidifierturnon)

    #humidifier.turnoff()
    humidifierturnoff = Functional_Script_Sentence("humidifier","turn","off",["humidifier", "moisturizer", "humidification device"],
                  [],
                  ["end", "finish", "power off", "sleep"],
                  ["turn"],["off"])
    funtion_list.append(humidifierturnoff)

    #----------------------------airpurifier-----------------------------------#
    #airpurifier.turnon()
    airpurifierturnon = Functional_Script_Sentence("airpurifier","turn","on",["air purifier", "air cleaner", "air filter"],
                  [],
                  ["start", "power on", "wake", "wake up"],
                  ["turn"],["on"])
    funtion_list.append(airpurifierturnon)

    #airpurifier.turnoff()
    airpurifierturnoff = Functional_Script_Sentence("airpurifier","turn","off",["air purifier", "air cleaner", "air filter"],
                  [],
                  ["end", "finish", "power off", "sleep"],
                  ["turn"],["off"])
    funtion_list.append(airpurifierturnoff)

    #----------------------------fan-----------------------------------#
    #fan.turnon()
    fanturnon = Functional_Script_Sentence("fan","turn","on",["fan", "air circulator", "blower"],
                  [],
                  ["start", "power on", "wake", "wake up"],
                  ["turn"],["on"])
    funtion_list.append(fanturnon)

    #fan.turnoff()
    fanturnoff = Functional_Script_Sentence("fan","turn","off",["fan", "air circulator", "blower"],
                  [],
                  ["end", "finish", "power off", "sleep"],
                  ["turn"],["off"])
    funtion_list.append(fanturnoff)

#==============================functional sentences==============================#

    operand_list = []
    
    whiteledlight_brightness = operand_container("whiteledlight.brightness",
                            ["main light's brightness","brightness of main light","white light's brightness",
                            "brightness of white light","white led's brightness","brightness of white led",
                            "led lamp's brightness","brightness of led lamp"],
                            ["equal or dimmer than", "no brighter than", "not brighter than", "equal or darker than"],
                            ["equal or brighter than", "no darker than", "not darker than", "no dimmer than", "not dimmer than"],
                            ["dimmer than","darker than"],
                            ["brighter than"])
    operand_list.append(whiteledlight_brightness)
                            
    redledlight_brightness = operand_container("redledlight.brightness",
                            ["red light's brightness","brightness of red light","alarm light's brightness",
                            "brightness of alarm light","red led's brightness","brightness of red led",
                            "red lamp's brightness","brightness of red lamp"],
                            ["equal or dimmer than", "no brighter than", "not brighter than", "equal or darker than"],
                            ["equal or brighter than", "no darker than", "not darker than", "no dimmer than", "not dimmer than"],
                            ["dimmer than","darker than"],
                            ["brighter than"])
    operand_list.append(redledlight_brightness)
    
    greenledlight_brightness = operand_container("greenledlight.brightness",
                            ["green light's brightness","brightness of green light",
                            "green led's brightness","brightness of green led",
                            "green lamp's brightness","brightness of green lamp"],
                            ["equal or dimmer than", "no brighter than", "not brighter than", "equal or darker than"],
                            ["equal or brighter than", "no darker than", "not darker than", "no dimmer than", "not dimmer than"],
                            ["dimmer than","darker than"],
                            ["brighter than"])
    operand_list.append(greenledlight_brightness)
                                                    
    blueledlight_brightness = operand_container("blueledlight.brightness",
                            ["blue light's brightness","brightness of blue light",
                            "blue led's brightness","brightness of blue led",
                            "blue lamp's brightness","brightness of blue lamp"],
                            ["equal or dimmer than", "no brighter than", "not brighter than", "equal or darker than"],
                            ["equal or brighter than", "no darker than", "not darker than", "no dimmer than", "not dimmer than"],
                            ["dimmer than","darker than"],
                            ["brighter than"])
    operand_list.append(blueledlight_brightness)
    
    powermeter_watt = operand_container("powermeter.watt",
                            ["watt","watt from power meter",
                            "measured watt","measured power"],
                            ["equal or lower than", "no higher than", "not higher than"],
                            ["equal or higher than", "no lower than", "not lower than"],
                            ["lower than"],
                            ["higher than"])
    operand_list.append(powermeter_watt)
                            
    scale_weight = operand_container("scale.weight",
                            ["weight","weight measured by scale",
                            "scale measured weight"],
                            ["equal or lower than", "equal or lighter than", "no higher than", "not higher than",
                            "no heavier than", "not heavier than", ],
                            ["equal or higher than", "equal or heavier than", "no lower than", "not lower than",
                            "no lighter than", "not lighter than", ],
                            ["lower than", "lighter than"],
                            ["higher than", "heavier than"])
    operand_list.append(scale_weight)
                            
    lightsensor_brightness = operand_container("lightsensor.brightness",
                            ["environmental brightness", "surrounding brightness", "ambient brightness",
                            "environmental light", "surrounding light", "ambient light"
                            "light sensor measured brightness", "brightness measured by light sensor",
                            "brightness by light sensor"  ],
                            ["equal or dimmer than", "no brighter than", "not brighter than", "equal or darker than"],
                            ["equal or brighter than", "no darker than", "not darker than", "no dimmer than", "not dimmer than"],
                            ["dimmer than","darker than"],
                            ["brighter than"])
    operand_list.append(lightsensor_brightness)
                            
    humidsensor_humidity = operand_container("humidsensor.humidity",
                            ["humidity", "room humidity", "humid sensor measured humidity",
                            "humidity by humid sensor", "humidity measured by humid sensor"],
                            ["equal or lower than", "equal or drier than", "no damper than", "not damper than"],
                            ["equal or damper than", "equal or higher than", "no drier than", "not drier than"],
                            ["drier than"],
                            ["damper than"])
    operand_list.append(humidsensor_humidity)
                            
    temperaturesensor_degree = operand_container("temperaturesensor.degree",
                            ["degree", "celsius", "temperature",
                            "degree by temperature sensor", "degree measured by temperature sensor",
                            "celsius by temperature sensor", "celsius measured by temperature sensor"],
                            ["equal or lower than", "equal or cooler than","equal or colder than", "no hotter than",
                            "not hotter than", "no warmer than", "not warmer than"],
                            ["equal or hotter than", "equal or higher than", "equal or warmer than", "no colder than",
                            "not colder than", "not cooler than", "not cooler than"],
                            ["cooler than", "colder than"],
                            ["hotter than", "warmer than"])
    operand_list.append(temperaturesensor_degree)
                            
    supersonicsensor_distance = operand_container("supersonicsensor.distance",
                            ["distance", "distance to object", "distance by supersonic sensor",
                            "distance measured by supersonic sensor"],
                            ["equal or closer than","no farther than", "not farther than"],
                            ["equal or farther than", "no closer than","not closer than"],
                            ["closer than"],
                            ["farther than"])
    operand_list.append(supersonicsensor_distance)
    
    time_hour = operand_container("time.hour",
                            ["hour", "clock hour"],
                            ["equal or earlier than","equal or faster than","no later than", "not later than"],
                            ["equal or later than", "no faster than","not faster than", "no earlier than","not earlier than"],
                            ["earlier than", "faster than"],
                            ["later than"])
    operand_list.append(time_hour)

    time_minute = operand_container("time.minute",
                            ["minute", "clock minute"],
                            ["equal or earlier than","equal or faster than","no later than", "not later than"],
                            ["equal or later than", "no faster than","not faster than", "no earlier than","not earlier than"],
                            ["earlier than", "faster than"],
                            ["later than"])
    operand_list.append(time_minute)
                            
    speaker_volume = operand_container("speaker.volume",
                            ["speaker volume","volume of speaker",
                            "speaker loudness","loudness of speaker",
                            "speaker sound volume","sound volume of speaker"],
                            ["equal or lower than", "equal or quieter than", "equal or smaller than",
                            "no higher than", "not higher than", "no larger than", "not larger than",
                            "no louder than", "not louder than"],
                            ["equal or higher than", "equal or larger than", "equal or louder than",
                            "no lower than", "not lower than", "no quieter than", "not quieter than",
                            "no smaller than", "not smaller than"],
                            ["lower than", "quieter than", "smaller than"],
                            ["higher than", "larger than", "louder than"])
    operand_list.append(speaker_volume)

    TV_volume = operand_container("TV.volume",
                            ["TV volume","volume of TV", "television volume","volume of television",
                            "TV loudness","loudness of TV", "television loudness","loudness of television",
                            "TV sound volume","sound volume of TV","television sound volume","sound volume of television"],
                            ["equal or lower than", "equal or quieter than", "equal or smaller than",
                            "no higher than", "not higher than", "no larger than", "not larger than",
                            "no louder than", "not louder than"],
                            ["equal or higher than", "equal or larger than", "equal or louder than",
                            "no lower than", "not lower than", "no quieter than", "not quieter than",
                            "no smaller than", "not smaller than"],
                            ["lower than", "quieter than", "smaller than"],
                            ["higher than", "larger than", "louder than"])
    operand_list.append(TV_volume)
                            
    TV_channel = operand_container("TV.channel",
                            ["TV channel","channel of TV", "television channel","channel of television"],
                            ["equal or lower than","no higher than", "not higher than"],
                            ["equal or higher than","no lower than", "not lower than"],
                            ["lower than"],
                            ["higher than"])
    operand_list.append(TV_channel)
    
    int_container_999 = []
    for i in range(100):
        temp_int_container =operand_container(str(i),[str(i)])
        int_container_999.append(temp_int_container)
    operator_list = ["==","!=","<=",">=",">","<"]
    cycle_unit = ["SEC","MIN","HOUR","DAY"]

#==============================trigger & loop generator==============================#

    f_script= open("script.txt","w+")
    f_sentence= open("sentence.txt","w+")
    total_functional_sentence_list_so = []
    total_functional_sentence_list_v = []
    
    operand_list_len = len(operand_list)
    operator_list_len = len(operator_list)
    
    for item in funtion_list:
        new_generator = Sentence_generator(item)
        new_generator.make_all()
        for so_sentence in item.complete_sentence_so:
            total_functional_sentence_list_so.append([item.script,so_sentence])
        for v_sentence in item.complete_sentence_v:
            total_functional_sentence_list_v.append([item.script,v_sentence])
    total_functional_sentence_list_so_len = len(total_functional_sentence_list_so)
    total_functional_sentence_list_v_len = len(total_functional_sentence_list_v)
        
    if ((os.path.exists("pure_script.txt")==False) and (os.path.exists("pure_sentence.txt"))==False):

        print("start building pure corpus")
        f_pure_script= open("pure_script.txt","w+")
        f_pure_sentence= open("pure_sentence.txt","w+")
        for item in total_functional_sentence_list_so:
            f_pure_script.write(repr(item[0])[1:-1])
            f_pure_sentence.write(item[1])
            f_pure_script.write("\n")
            f_pure_sentence.write("\n")
        for item in total_functional_sentence_list_v:
            f_pure_script.write(repr(item[0])[1:-1])
            f_pure_sentence.write(item[1])
            f_pure_script.write("\n")
            f_pure_sentence.write("\n")
        f_pure_script.close()
        f_pure_sentence.close()
    
    if ((os.path.exists("trigger_script.txt")==False) and (os.path.exists("trigger_script.txt"))==False):
        f_trigger_script= open("trigger_script.txt","w+")
        f_trigger_sentence= open("trigger_sentence.txt","w+")
        print("start building trigger corpus")
        for i in range(50000):
            print(str(i+1) + "/50000")
            temp_wait1 = Conditional_Script_Sentence("wait until", operand_list[i%operand_list_len], operator_list[i%operator_list_len],int_container_999[i%100], None, None, None, None)
            condition1_generator(temp_wait1)
            f_trigger_script.write(repr(temp_wait1.script +"\n{")[1:-1])
            f_trigger_sentence.write(random.choice(temp_wait1.complete_phrase))
            f_trigger_script.write("\n")
            f_trigger_sentence.write("\n")
            
            temp_wait2 = Conditional_Script_Sentence("wait until", operand_list[(i+1)%operand_list_len], operator_list[(i+1)%operator_list_len],int_container_999[i%100], "and", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            condition2_generator(temp_wait2)
            f_trigger_script.write(repr(temp_wait2.script +"\n{")[1:-1])
            f_trigger_sentence.write(random.choice(temp_wait2.complete_phrase))
            f_trigger_script.write("\n")
            f_trigger_sentence.write("\n")

            temp_wait3 = Conditional_Script_Sentence("wait until", operand_list[(i+2)%operand_list_len], operator_list[(i+2)%operator_list_len],int_container_999[i%100], "or", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            condition2_generator(temp_wait3)
            f_trigger_script.write(repr(temp_wait3.script +"\n{")[1:-1])
            f_trigger_sentence.write(random.choice(temp_wait3.complete_phrase))
            f_trigger_script.write("\n")
            f_trigger_sentence.write("\n")

            temp_if1 = Conditional_Script_Sentence("if", operand_list[(i+3)%operand_list_len], operator_list[(i+3)%operator_list_len],int_container_999[i%100], None, None, None, None)
            condition1_generator(temp_if1)
            f_trigger_script.write(repr(temp_if1.script +"\n{")[1:-1])
            f_trigger_sentence.write(random.choice(temp_if1.complete_phrase))
            f_trigger_script.write("\n")
            f_trigger_sentence.write("\n")

            temp_if2 = Conditional_Script_Sentence("if", operand_list[(i+4)%operand_list_len], operator_list[(i+4)%operator_list_len],int_container_999[i%100], "or", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            condition2_generator(temp_if2)
            f_trigger_script.write(repr(temp_if2.script +"\n{")[1:-1])
            f_trigger_sentence.write(random.choice(temp_if2.complete_phrase))
            f_trigger_script.write("\n")
            f_trigger_sentence.write("\n")

            temp_if3 = Conditional_Script_Sentence("if", operand_list[(i+5)%operand_list_len], operator_list[(i+5)%operator_list_len],int_container_999[i%100], "and", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            condition2_generator(temp_if3)
            f_trigger_script.write(repr(temp_if3.script +"\n{")[1:-1])
            f_trigger_sentence.write(random.choice(temp_if3.complete_phrase))
            f_trigger_script.write("\n")
            f_trigger_sentence.write("\n")

            temp_loop1 = Loop_Script_Sentence(i%60+1, random.choice(cycle_unit), operand_list[(i+6)%operand_list_len], operator_list[(i+6)%operator_list_len],int_container_999[i%100], None, None, None, None)
            loop1_generator(temp_loop1)
            for item in temp_loop1.complete_phrase:
                f_trigger_script.write(repr(temp_loop1.script +"\n{")[1:-1])
                f_trigger_sentence.write(item)
                f_trigger_script.write("\n")
                f_trigger_sentence.write("\n")

        f_trigger_script.close()
        f_trigger_sentence.close()
 
    

    print("start building exhustive search corpus")
    while(False):
        print (str(i+1) + "/5000000")
        trigger_phrase_list = []
        loop_phrase_list = []
        temp_wait1 = Conditional_Script_Sentence("wait until", operand_list[i%operand_list_len], operator_list[i%operator_list_len],int_container_999[i%100], None, None, None, None)
        condition1_generator(temp_wait1)
        
        trigger_phrase_list.append([temp_wait1.script,random.choice(temp_wait1.complete_phrase)])
        
        temp_wait2 = Conditional_Script_Sentence("wait until", operand_list[(i+1)%operand_list_len], operator_list[(i+1)%operator_list_len],int_container_999[i%100], "and", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
        condition2_generator(temp_wait2)
        trigger_phrase_list.append([temp_wait2.script,random.choice(temp_wait2.complete_phrase)])

        temp_wait3 = Conditional_Script_Sentence("wait until", operand_list[(i+2)%operand_list_len], operator_list[(i+2)%operator_list_len],int_container_999[i%100], "or", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
        condition2_generator(temp_wait3)
        trigger_phrase_list.append([temp_wait3.script,random.choice(temp_wait3.complete_phrase)])

        temp_if1 = Conditional_Script_Sentence("if", operand_list[(i+3)%operand_list_len], operator_list[(i+3)%operator_list_len],int_container_999[i%100], None, None, None, None)
        condition1_generator(temp_if1)
        trigger_phrase_list.append([temp_if1.script,random.choice(temp_if1.complete_phrase)])

        temp_if2 = Conditional_Script_Sentence("if", operand_list[(i+4)%operand_list_len], operator_list[(i+4)%operator_list_len],int_container_999[i%100], "or", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
        condition2_generator(temp_if2)
        trigger_phrase_list.append([temp_if2.script,random.choice(temp_if2.complete_phrase)])
        
        temp_if3 = Conditional_Script_Sentence("if", operand_list[(i+5)%operand_list_len], operator_list[(i+5)%operator_list_len],int_container_999[i%100], "and", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
        condition2_generator(temp_if3)
        trigger_phrase_list.append([temp_if3.script,random.choice(temp_if3.complete_phrase)])
        
        temp_loop1 = Loop_Script_Sentence(i%60+1, random.choice(cycle_unit), operand_list[(i+6)%operand_list_len], operator_list[(i+6)%operator_list_len],int_container_999[i%100], None, None, None, None)
        loop1_generator(temp_loop1)
        for item in temp_loop1.complete_phrase:
            loop_phrase_list.append([temp_loop1.script,item])
            print(temp_loop1.script)
            print(item)
            
#            temp_loop2 = Loop_Script_Sentence(i%60+1, random.choice(cycle_unit) operand_list[(i+7)%operand_list_len], operator_list[(i+7)%operator_list_len],int_container_999[i], "and", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
#            loop1_generator(temp_loop2)
#            for item in temp_loop2.complete_phrase:
#                loop_phrase_list.append([temp_loop2.script,item])
#
#            temp_loop3 = Loop_Script_Sentence(str(random.randint(1,99)), random.choice(cycle_unit),random.choice(operand_list), random.choice(operator_list),random.choice(int_container_999), "or", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
#            loop2_generator(temp_loop3)
#            for item in temp_loop3.complete_phrase:
#                loop_phrase_list.append([temp_loop3.script,item])
    #==============================trigger & loop generator==============================#
        
        for temp_trigger in trigger_phrase_list:
            total_functional_sentence_list_so[i%total_functional_sentence_list_so_len][0]
            temp_so = random.choice(total_functional_sentence_list_so)
            temp_v = random.choice(total_functional_sentence_list_v)
            samplescrpit = temp_trigger[0] + "\n{\n" + total_functional_sentence_list_so[i%total_functional_sentence_list_so_len][0] + "\n}"
            samplesentence = temp_trigger[1] + " and then make " + total_functional_sentence_list_so[i%total_functional_sentence_list_so_len][1]
            f_script.write(repr(samplescrpit)[1:-1])
            f_sentence.write(samplesentence)
            f_script.write("\n")
            f_sentence.write("\n")
            
            samplescrpit = temp_trigger[0] + "\n{\n" + total_functional_sentence_list_so[i%total_functional_sentence_list_so_len][0] + "\n}"
            samplesentence = temp_trigger[1] + " then make " + total_functional_sentence_list_so[i%total_functional_sentence_list_so_len][1]
            f_script.write(repr(samplescrpit)[1:-1])
            f_sentence.write(samplesentence)
            f_script.write("\n")
            f_sentence.write("\n")
            
            samplescrpit = temp_trigger[0] + "\n{\n" + total_functional_sentence_list_so[i%total_functional_sentence_list_v_len][0] + "\n}"
            samplesentence = temp_trigger[1] + " and then " + total_functional_sentence_list_so[i%total_functional_sentence_list_v_len][1]
            f_script.write(repr(samplescrpit)[1:-1])
            f_sentence.write(samplesentence)
            f_script.write("\n")
            f_sentence.write("\n")
            
        for temp_loop in loop_phrase_list:
            samplescrpit = temp_loop[0] + "\n{\n" + total_functional_sentence_list_so[i%total_functional_sentence_list_so_len][0] + "\n}"
            samplesentence = total_functional_sentence_list_so[i%total_functional_sentence_list_so_len][1] + " " + temp_loop[1]
            f_script.write(repr(samplescrpit)[1:-1])
            f_sentence.write(samplesentence)
            f_script.write("\n")
            f_sentence.write("\n")
            
    print("start building random search corpus")
    for i in range(15000):
        print (str(i+1) + "/15000")
        trigger_phrase_list = []
        loop_phrase_list = []
        for j in range(20):
            temp_wait1 = Conditional_Script_Sentence("wait until", random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], None, None, None, None)
            condition1_generator(temp_wait1)
            for item in temp_wait1.complete_phrase:
                trigger_phrase_list.append([temp_wait1.script,item])

            temp_wait2 = Conditional_Script_Sentence("wait until", random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], "and", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            condition2_generator(temp_wait2)
            for item in temp_wait2.complete_phrase:
                trigger_phrase_list.append([temp_wait2.script,item])

            temp_wait3 = Conditional_Script_Sentence("wait until", random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], "or", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            condition2_generator(temp_wait3)
            for item in temp_wait3.complete_phrase:
                trigger_phrase_list.append([temp_wait3.script,item])

            temp_if1 = Conditional_Script_Sentence("if", random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], None, None, None, None)
            condition1_generator(temp_if1)
            for item in temp_if1.complete_phrase:
                trigger_phrase_list.append([temp_if1.script,item])

            temp_if2 = Conditional_Script_Sentence("if", random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], "or", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            condition2_generator(temp_if2)
            for item in temp_if2.complete_phrase:
                trigger_phrase_list.append([temp_if2.script,item])
                 

            temp_if3 = Conditional_Script_Sentence("if", random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], "and", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            condition2_generator(temp_if3)
            for item in temp_if3.complete_phrase:
                trigger_phrase_list.append([temp_if3.script,item])

            temp_loop1 = Loop_Script_Sentence(i%60+1, random.choice(cycle_unit),random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], None, None, None, None)
            loop1_generator(temp_loop1)
            for item in temp_loop1.complete_phrase:
                loop_phrase_list.append([temp_loop1.script,item])

            temp_loop2 = Loop_Script_Sentence(i%60+1, random.choice(cycle_unit),random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], "and", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            loop1_generator(temp_loop2)
            for item in temp_loop2.complete_phrase:
                loop_phrase_list.append([temp_loop2.script,item])
#
#            temp_loop3 = Loop_Script_Sentence(str(random.randint(1,99)), random.choice(cycle_unit),random.choice(operand_list), random.choice(operator_list),random.choice(int_container_999), "or", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
#            loop2_generator(temp_loop3)
#            for item in temp_loop3.complete_phrase:
#                loop_phrase_list.append([temp_loop3.script,item])

    #==============================trigger & loop generator==============================#

        for k in range(1000):
            temp_trigger = random.choice(trigger_phrase_list)
            temp_loop = random.choice(loop_phrase_list)
            temp_so = random.choice(total_functional_sentence_list_so)
            temp_v = random.choice(total_functional_sentence_list_v)
            samplescrpit = ""
            samplesentence = ""
            if k%5 == 0:
                samplescrpit = temp_trigger[0] + "\n{\n" + temp_so[0] + "\n}"
                samplesentence = temp_trigger[1] + " and then make " + temp_so[1]
            elif k%5 == 1:
                samplescrpit = temp_trigger[0] + "\n{\n" + temp_v[0] + "\n}"
                samplesentence = temp_trigger[1] + " and then " + temp_v[1]
            elif k%5 == 2:
                samplescrpit = temp_trigger[0] + "\n{\n" + temp_so[0] + "\n}"
                samplesentence = temp_trigger[1] + " then make " + temp_so[1]
            elif k%5 == 3:
                samplescrpit = temp_loop[0] + "\n{\n" + temp_so[0] + "\n}"
                samplesentence = temp_so[1] + " " + temp_loop[1]
            elif k%5 == 4:
                samplescrpit = temp_loop[0] + "\n{\n" + temp_v[0] + "\n}"
                samplesentence = temp_v[1] + " " +temp_loop[1]

            f_script.write(repr(samplescrpit)[1:-1])
            f_sentence.write(samplesentence)
            f_script.write("\n")
            f_sentence.write("\n")
    f_script.close()
    f_sentence.close()
            
    f_script= open("test.scrpit","w+")
    f_sentence= open("test.sen","w+")
    print("start building test set")
    for i in range(1000):
        print (str(i+1) + "/1000")
        trigger_phrase_list = []
        loop_phrase_list = []
        for j in range(20):
            temp_wait1 = Conditional_Script_Sentence("wait until", random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], None, None, None, None)
            condition1_generator(temp_wait1)
            for item in temp_wait1.complete_phrase:
                trigger_phrase_list.append([temp_wait1.script,item])

            temp_wait2 = Conditional_Script_Sentence("wait until", random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], "and", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            condition2_generator(temp_wait2)
            for item in temp_wait2.complete_phrase:
                trigger_phrase_list.append([temp_wait2.script,item])

            temp_wait3 = Conditional_Script_Sentence("wait until", random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], "or", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            condition2_generator(temp_wait3)
            for item in temp_wait3.complete_phrase:
                trigger_phrase_list.append([temp_wait3.script,item])

            temp_if1 = Conditional_Script_Sentence("if", random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], None, None, None, None)
            condition1_generator(temp_if1)
            for item in temp_if1.complete_phrase:
                trigger_phrase_list.append([temp_if1.script,item])

            temp_if2 = Conditional_Script_Sentence("if", random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], "or", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            condition2_generator(temp_if2)
            for item in temp_if2.complete_phrase:
                trigger_phrase_list.append([temp_if2.script,item])
                 

            temp_if3 = Conditional_Script_Sentence("if", random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], "and", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            condition2_generator(temp_if3)
            for item in temp_if3.complete_phrase:
                trigger_phrase_list.append([temp_if3.script,item])

            temp_loop1 = Loop_Script_Sentence(i%60+1, random.choice(cycle_unit),random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], None, None, None, None)
            loop1_generator(temp_loop1)
            for item in temp_loop1.complete_phrase:
                loop_phrase_list.append([temp_loop1.script,item])

            temp_loop2 = Loop_Script_Sentence(i%60+1, random.choice(cycle_unit),random.choice(operand_list), random.choice(operator_list),int_container_999[i%100], "and", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
            loop1_generator(temp_loop2)
            for item in temp_loop2.complete_phrase:
                loop_phrase_list.append([temp_loop2.script,item])
#
#            temp_loop3 = Loop_Script_Sentence(str(random.randint(1,99)), random.choice(cycle_unit),random.choice(operand_list), random.choice(operator_list),random.choice(int_container_999), "or", random.choice(operand_list), random.choice(operator_list), random.choice(int_container_999))
#            loop2_generator(temp_loop3)
#            for item in temp_loop3.complete_phrase:
#                loop_phrase_list.append([temp_loop3.script,item])

    #==============================trigger & loop generator==============================#

        for k in range(200):
            temp_trigger = random.choice(trigger_phrase_list)
            temp_loop = random.choice(loop_phrase_list)
            temp_so = random.choice(total_functional_sentence_list_so)
            temp_v = random.choice(total_functional_sentence_list_v)
            samplescrpit = ""
            samplesentence = ""
            if k%5 == 0:
                samplescrpit = temp_trigger[0] + "\n{\n" + temp_so[0] + "\n}"
                samplesentence = temp_trigger[1] + " and then make " + temp_so[1]
            elif k%5 == 1:
                samplescrpit = temp_trigger[0] + "\n{\n" + temp_v[0] + "\n}"
                samplesentence = temp_trigger[1] + " and then " + temp_v[1]
            elif k%5 == 2:
                samplescrpit = temp_trigger[0] + "\n{\n" + temp_so[0] + "\n}"
                samplesentence = temp_trigger[1] + " then make " + temp_so[1]
            elif k%5 == 3:
                samplescrpit = temp_loop[0] + "\n{\n" + temp_so[0] + "\n}"
                samplesentence = temp_so[1] + " " + temp_loop[1]
            elif k%5 == 4:
                samplescrpit = temp_loop[0] + "\n{\n" + temp_v[0] + "\n}"
                samplesentence = temp_v[1] + " " +temp_loop[1]

            f_script.write(repr(samplescrpit)[1:-1])
            f_sentence.write(samplesentence)
            f_script.write("\n")
            f_sentence.write("\n")
    f_script.close()
    f_sentence.close()
