from collections import namedtuple


Keyframe = namedtuple("Keyframe", "keyframe transform")
Sequence = namedtuple("Sequence", "name sequence subSequences frameTriggers")
SubSequence = namedtuple("SubSequence", "subSequence sequence keyframes")
Object = namedtuple("Object", "name object node mesh sequences subSequences")
Node = namedtuple("Node", "name node defaultTransform sequences subSequences parentNode childNodes object")
Detail = namedtuple("Detail", "detail rootNode")

def mapObjects(shape, shouldFail):
    mappedSequences = []
    mappedSubSequences = []
    mappedNodes = []
    mappedObjects = []
    mappedKeyframes = []
    mappedDetails = []
    nodeDict = {}

    for keyframe in shape.keyframes:
        if keyframe.fKeyValue > len(shape.transforms) - 1:
            error = "Shape does not have the correct number of transforms. Has " + str(len(shape.transforms)) + ", needs at least " + str(keyframe.fKeyValue + 1)
            if shouldFail:
                raise ValueError(error)
            else:
                print error

        if keyframe.fKeyValue < len(shape.transforms) - 1:
            someTransform = shape.transforms[keyframe.fKeyValue]
            mappedKeyframes.append(Keyframe._make((keyframe, someTransform)))

    for sequence in shape.sequences:
        sequenceName = shape.names[sequence.fName].name
        sequenceName = sequenceName.split("\0")[0]
        frameTrigs = []
        i = 0
        while i < sequence.fNumFrameTriggers:
            index = sequence.fFirstFrameTrigger + i
            frameTrigs.append(shape.frameTriggers[index])
            i += 1
        mappedSequences.append(Sequence._make((sequenceName, sequence, [], frameTrigs)))

    for subSequence in shape.subSequences:
        sequence = mappedSequences[subSequence.fSequenceIndex]
        keyFrams = []
        i = 0
        while i < subSequence.fnKeyframes:
            index = subSequence.fFirstKeyframe + i
            if index > len(mappedKeyframes) - 1:
                error = "Shape does not have the correct number of mapped keyframes. Has " + str(len(mappedKeyframes)) + ", should have " + str(subSequence.fFirstKeyframe + subSequence.fnKeyframes - 1)
                if shouldFail:
                    raise ValueError(error)
                else:
                    print error
            if index < len(mappedKeyframes) - 1:
                keyFrams.append(mappedKeyframes[index])
            i += 1

        subSeq = SubSequence._make((subSequence, sequence, keyFrams))
        sequence.subSequences.append(subSeq)
        mappedSubSequences.append(subSeq)

    for node in shape.nodes:
        nodeName = shape.names[node.fName].name
        nodeName = nodeName.split("\0")[0]
        someTransform = shape.transforms[node.fDefaultTransform]
        seqs = {}
        subSeqs = []
        i = 0
        while i < node.fnSubSequences:
            index = node.fFirstSubSequence + i
            subSeq = mappedSubSequences[index]
            if subSeq.sequence.name not in seqs:
                seqs[subSeq.sequence.name] = subSeqs
            subSeqs.append(subSeq)
            i += 1

        parentNode = None
        if node.fParent > -1:
            parentNode = mappedNodes[node.fParent]

        object = None
        finalNode = Node._make((nodeName, node, someTransform, seqs, subSeqs, parentNode, {}, {"instance" : None}))

        if node.fParent > -1:
            parentNode.childNodes[finalNode.name] = finalNode

        nodeDict[finalNode.name] = finalNode
        mappedNodes.append(finalNode)

    for object in shape.objects:
        someNode = mappedNodes[object.fNodeIndex]
        someObjectName = shape.names[object.fName].name
        someObjectName = someObjectName.split("\0")[0]
        someMesh = shape.meshes[object.fMeshIndex]
        seqs = {}
        subSeqs = []
        i = 0
        while i < object.fnSubSequences:
            index = object.fFirstSubSequence + i
            subSeq = mappedSubSequences[index]
            if subSeq.sequence.name not in seqs:
                seqs[subSeq.sequence.name] = subSeqs
            subSeqs.append(subSeq)
            i += 1

        finalObject = Object._make((someObjectName, object, someNode, someMesh, seqs, subSeqs))
        someNode.object["instance"] = finalObject
        mappedObjects.append(finalObject)

    for detail in shape.details:
        someNode = mappedNodes[detail.fRootNodeIndex]
        if len(mappedNodes) < len(mappedObjects):
            for object in mappedObjects:
                if object.name not in someNode.childNodes:

                    finalNode = Node._make((object.name, None, None, object.sequences, object.subSequences, someNode, {}, {"instance" : object}))
                    someNode.childNodes[object.name] = finalNode

        mappedDetails.append(Detail._make((detail, someNode)))
    return mappedDetails
