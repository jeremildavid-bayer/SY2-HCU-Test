import io, sys, json, chardet, codecs

# Minimum Python3.5 should be used
assert sys.version_info >= (3,5)

filename=sys.argv[1]
print("Processing file : " + filename)

# Files types are utf-8 with BOM
origFile = io.open(filename, encoding='utf-8-sig')
origData = json.load(origFile)
origFile.close()

phrases = origData["Phrases"]

# Remove unused fields to optimize size
for phrase in phrases:
    phrases[phrase].pop('SourceCultureCode',None)
    phrases[phrase].pop('Source',None)
    phrases[phrase].pop('SourceAt',None)
    phrases[phrase].pop('TranslationCultureCode',None)
    phrases[phrase].pop('TranslatedAt',None)
    phrases[phrase].pop('ExampleContext',None)
    phrases[phrase].pop('UsedBy',None)


# This will dump minified 
newData = json.dumps(origData)

# Overwrite original file
newFile = codecs.open(filename, "w", "utf-8-sig")
newFile.write(newData)
newFile.close()
