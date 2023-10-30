def learnPredictor(trainExamples: List[Tuple[T, int]],
                   validationExamples: List[Tuple[T, int]],
                   featureExtractor: Callable[[T], FeatureVector],
                   numEpochs: int, eta: float) -> WeightVector:
    '''
    Given |trainExamples| and |validationExamples| (each one is a list of (x,y)
    pairs), a |featureExtractor| to apply to x, and the number of epochs to
    train |numEpochs|, the step size |eta|, return the weight vector (sparse
    feature vector) learned.

    Note:
    - The predictor should output +1 if the score is precisely 0.
    '''
    weights = {}  # feature => weight
        
    def predict(x):
        if dotProduct(featureExtractor(x), weights) >= 0:
            return 1
        else:
            return -1
    
    # For each epoch, compute the gradient
    for epoch in range(numEpochs):
        
        for x,y in trainExamples:
            gradient = {}
            
            # Extract word features (phi(x))
            features = extractWordFeatures(x)
            
            # Compute margin
            margin = dotProduct(weights, features)*y # w.phi(x) * y
            
            # If (1-margin) > 0 compute gradient, else gradient = 0
            for word in features:
                if (1 - margin) > 0:
                    gradient[word] = -features[word]*y # -phi(x)y
                else: 
                    gradient[word] = 0
                
            # Update the weight vector:
            # If word feature already in weight vector, update weight
            for word in features:
                if word in weights:
                    weights[word] = weights[word] - (eta * gradient[word])
                else:
                    weights[word] = - (eta * gradient[word])
                
        # Print results
        print('Epoch', epoch, 'Training Error is: ', evaluatePredictor(trainExamples, predict))
        print('Epoch', epoch, 'Validation Error is: ', evaluatePredictor(validationExamples, predict))
    return weights