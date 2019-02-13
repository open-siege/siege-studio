from collections import namedtuple


Keyframe = namedtuple("Keyframe", "keyframe transform")
Sequence = namedtuple("Sequence", "name sequence frameTriggers")
SubSequence = namedtuple("SubSequence", "subSequence sequence keyframes")
Object = namedtuple("Object", "name object node mesh subSequences")
Node = namedtuple("Node", "name node defaultTransform subSequences parentNode childNodes object")
Detail = namedtuple("Detail", "detail rootNode")

def mapObjects(shape):
    mappedSequences = []
    mappedSubSequences = []
    mappedNodes = []
    mappedObjects = []
    mappedKeyframes = []
    mappedDetails = []

    for keyframe in shape.keyframes:
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
        mappedSequences.append(Sequence._make((sequenceName, sequence, frameTrigs)))

    for subSequence in shape.subSequences:
        sequence = mappedSequences[subSequence.fSequenceIndex]
        keyFrams = []
        i = 0
        while i < subSequence.fnKeyframes:
            index = subSequence.fFirstKeyframe + i
            keyFrams.append(mappedKeyframes[index])
            i += 1

        mappedSubSequences.append(SubSequence._make((subSequence, sequence, keyFrams)))

    for node in shape.nodes:
        nodeName = shape.names[node.fName].name
        nodeName = nodeName.split("\0")[0]
        someTransform = shape.transforms[node.fDefaultTransform]
        subSeqs = []
        i = 0
        while i < node.fnSubSequences:
            index = node.fFirstSubSequence + i
            subSeqs.append(mappedSubSequences[index])
            i += 1

        parentNode = None
        if node.fParent > -1:
            parentNode = mappedNodes[node.fParent]

        finalNode = Node._make((nodeName, node, someTransform, subSeqs, parentNode, [], []))

        if node.fParent > -1:
            parentNode.childNodes.append(finalNode)

        mappedNodes.append(finalNode)


    for object in shape.objects:
        someNode = mappedNodes[object.fNodeIndex]
        someObjectName = shape.names[object.fName].name
        someObjectName = someObjectName.split("\0")[0]
        someMesh = shape.meshes[object.fMeshIndex]
        subSeqs = []
        i = 0
        while i < object.fnSubSequences:
            index = object.fFirstSubSequence + i
            subSeqs.append(mappedSubSequences[index])
            i += 1

        finalObject = Object._make((someObjectName, object, someNode, someMesh, subSeqs))
        someNode.object.append(finalObject)
        mappedObjects.append(finalObject)

    for detail in shape.details:
        someNode = mappedNodes[detail.fRootNodeIndex]
        mappedDetails.append(Detail._make((detail, someNode)))

    return mappedDetails
