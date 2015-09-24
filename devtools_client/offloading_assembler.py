'''
Created on Mar 2, 2015

@author: cjneasbi
'''
import ijson
import json
import sys
import decimal

data = {"recording": {}}
domains = ["blink", "v8"]

class DecimalEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, decimal.Decimal):
            return float(obj)
        
        return json.JSONEncoder.default(self, obj)

def main():
    if len(sys.argv) > 1:
        for filename in sys.argv[1:]:
            parseFile(filename, data["recording"])
    
    print json.dumps(data, sort_keys=True, indent=4, separators=(',', ':'), cls=DecimalEncoder)
    
def parseFile(filename, dataMap):
    with open(filename, 'r') as f:
        objs = ijson.items(f, "item.params")
        for obj in objs:
            objDomain = str(obj['domain'])
            if objDomain in domains:
                if objDomain not in dataMap:
                    dataMap[objDomain] = {}
                parseDomain(dataMap[objDomain], obj)
                    
def parseDomain(domainData, obj):
    dataStruct = str(obj["name"])
    replace = obj["replace"] if "replace" in obj else False
    key = None
    if "key" in obj and len(obj["key"]) > 0: #key is an optional property
        key = getRealValue(obj, "key")
    
    if replace and not key:
        domainData[dataStruct] = getRealValue(obj, "value")
    else:
        if dataStruct not in domainData: 
            domainData[dataStruct] = [] #maps are stored as arrays of (key, value) objects
        parseData(domainData[dataStruct], obj, key, replace)

def getRealValue(obj, key):
    if key in obj[key] and len(obj[key]) == 1:
        return obj[key][key] # non-object serialized values, i.e. single values or arrays
    else:
        return obj[key] # object serialized values

def parseData(buf, obj, key, replace):
    batchAdd = obj["batch"] if "batch" in obj else False            
    valueVal = getRealValue(obj, "value")
    
    if key:
        #map data structure
        parseMap(buf, key, valueVal, batchAdd, replace)
    else:
        #vector data structure
        parseVector(buf, valueVal, batchAdd)               
            
def parseMap(buf, keyVal, valueVal, batchAdd, replace):       
    entryDict = None
    for entry in buf:
        if entry["key"] == keyVal:
            entryDict = entry
            break

    if not entryDict:
        entryDict = {"key": keyVal, "value": []}
        buf.append(entryDict)

    if replace:
        entryDict["value"] = valueVal
    else:  
        if batchAdd and type(valueVal) == list:
            entryDict["value"].extend(valueVal)
        else:    
            entryDict["value"].append(valueVal)
                   
def parseVector(buf, valueVal, batchAdd):
    if batchAdd and type(valueVal) == list:
        buf.extend(valueVal)
    else:
        buf.append(valueVal)
        

if __name__ == '__main__':
    main()