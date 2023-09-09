"""
File: searchengine.py
---------------------
This search engine receives a user's query and searches throughout a list of files for
 documents that include all items in the user's query, and returns to the user the location
 of such files.
"""


import os
import sys
import string


def create_index(filenames, index, file_titles):
    """
    This function is passed:
        filenames:      a list of file names (strings)

        index:          a dictionary mapping from terms to file names (i.e., inverted index)
                        (term -> list of file names that contain that term)

        file_titles:    a dictionary mapping from a file names to the title of the article
                        in a given file
                        (file name -> title of article in that file)

    The function will update the index passed in to include the terms in the files
    in the list filenames.  Also, the file_titles dictionary will be updated to
    include files in the list of filenames.

    >>> index = {}
    >>> file_titles = {}
    >>> create_index(['test1.txt'], index, file_titles)
    >>> index
    {'file': ['test1.txt'], '1': ['test1.txt'], 'title': ['test1.txt'], 'apple': ['test1.txt'], 'ball': ['test1.txt'], 'carrot': ['test1.txt']}
    >>> file_titles
    {'test1.txt': 'File 1 Title'}
    >>> index = {}
    >>> file_titles = {}
    >>> create_index(['test2.txt'], index, file_titles)
    >>> index
    {'file': ['test2.txt'], '2': ['test2.txt'], 'title': ['test2.txt'], 'ball': ['test2.txt'], 'carrot': ['test2.txt'], 'dog': ['test2.txt']}
    >>> file_titles
    {'test2.txt': 'File 2 Title'}
    >>> index = {}
    >>> file_titles = {}
    >>> create_index(['test1.txt', 'test2.txt'], index, file_titles)
    >>> index
    {'file': ['test1.txt', 'test2.txt'], '1': ['test1.txt'], 'title': ['test1.txt', 'test2.txt'], 'apple': ['test1.txt'], 'ball': ['test1.txt', 'test2.txt'], 'carrot': ['test1.txt', 'test2.txt'], '2': ['test2.txt'], 'dog': ['test2.txt']}
    >>> index = {}
    >>> file_titles = {}
    >>> create_index(['test1.txt', 'test2.txt', 'test2.txt'], index, file_titles)
    >>> index
    {'file': ['test1.txt', 'test2.txt'], '1': ['test1.txt'], 'title': ['test1.txt', 'test2.txt'], 'apple': ['test1.txt'], 'ball': ['test1.txt', 'test2.txt'], 'carrot': ['test1.txt', 'test2.txt'], '2': ['test2.txt'], 'dog': ['test2.txt']}
    >>> file_titles
    {'test1.txt': 'File 1 Title', 'test2.txt': 'File 2 Title'}
    >>> index = {'file': ['test1.txt'], '1': ['test1.txt'], 'title': ['test1.txt'], 'apple': ['test1.txt'], 'ball': ['test1.txt'], 'carrot': ['test1.txt']}
    >>> file_titles = {'test1.txt': 'File 1 Title'}
    >>> create_index([], index, file_titles)
    >>> index
    {'file': ['test1.txt'], '1': ['test1.txt'], 'title': ['test1.txt'], 'apple': ['test1.txt'], 'ball': ['test1.txt'], 'carrot': ['test1.txt']}
    >>> file_titles
    {'test1.txt': 'File 1 Title'}
    """

    for file in filenames:
        get_title(file,file_titles)
        file_opened = open(file)        # read file and process every line
        for line in file_opened:    # loop through all the lines in the current file
            line = line.strip().strip(string.punctuation).split()   # format each line
            for word in line:           # loop through every word in each line,
                word = word.lower().strip().strip(string.punctuation)     # make word in lowercase and format
                if word not in index:           # if word is not in the index, add word as key and file name as value
                    index[word] = [file]
                if file not in index[word]:     # add file name as value if it does not exist in key [word]
                    index[word] += [file]


def get_title(file,file_titles):
    """
    This function is passed a file and a dictionary and adds the title of the file
    as an value for the key (file name) in the file_titles dictionary.
    Function returns dictionary populated with the file title.
    """
    title_file = open(file)                                             # read file and save title
    title = title_file.readline().strip().strip(string.punctuation)     # remove punctuation and white space
    file_titles[file] = title                                           # add file title to dictionary file_titles
    return(file_titles)


def search(index, query):
    """
    This function is passed:
        index:      a dictionary mapping from terms to file names (inverted index)
                    (term -> list of file names that contain that term)

        query  :    a query (string), where any letters will be lowercase

    The function returns a list of the names of all the files that contain *all* of the
    terms in the query (using the index passed in).

    >>> index = {}
    >>> create_index(['test1.txt', 'test2.txt'], index, {})
    >>> search(index, 'apple')
    ['test1.txt']
    >>> search(index, 'ball')
    ['test1.txt', 'test2.txt']
    >>> search(index, 'file')
    ['test1.txt', 'test2.txt']
    >>> search(index, '2')
    ['test2.txt']
    >>> search(index, 'carrot')
    ['test1.txt', 'test2.txt']
    >>> search(index, 'dog')
    ['test2.txt']
    >>> search(index, 'nope')
    []
    >>> search(index, 'apple carrot')
    ['test1.txt']
    >>> search(index, 'apple ball file')
    ['test1.txt']
    >>> search(index, 'apple ball nope')
    []
    """
    """This is what I included in previous submission, it works as -OR- (all items)"""
    # file_names = []
    # query_words = query.split()
    # for term in query_words:
    #     if term in index:       # Check if word exists (as key) in index
    #         posting_list = index[term]
    #
    #         for file in posting_list:
    #             if file not in file_names:
    #                 file_names.append(file)
    #         posting_list.clear()
    # return file_names

    """This is what I was trying to implement for queries with +1 word, which should work as -AND- (only common items)"""
    files = []
    query_words = query.split()
    check_terms = check_all_terms(query_words,index)
    if check_terms == True:
        for term in query_words:
            if len(query_words) == 0:
                files = index[term]  # Populate files list with files that contain first word
            else:
                posting_list = index[term]
                if len(files) == 0:
                    files = posting_list
                else:
                    common_elements(files, posting_list)
    return files


def common_elements(files,posting_list):
    """
    This function gets passed two lists and returns a list of common elements.
    """
    for file in files:
        for new_file in posting_list:
            files.clear()
            if new_file == file and file not in files and new_file not in files:
                files.append(new_file)
    return files


def check_all_terms(query_words,index):
    """
    This function gets passed the user's query as a list of words and an index, which contains all the words in the
    engine as keys and the files that contain such word, as values.
    If all the terms in the user's query are found in the index, it returns TRUE, otherwise, false.
    This is used to truncate the search function if one of the terms in the query does not exist in the index,
    and as a consequence, no results are found for the specific query.
    """
    if all(term in index for term in query_words):
        return True
    else:
        return False


##### YOU SHOULD NOT NEED TO MODIFY ANY CODE BELOW THIS LINE (UNLESS YOU'RE ADDING EXTENSIONS) #####


def do_searches(index, file_titles):
    """
    This function is given an inverted index and a dictionary mapping from
    file names to the titles of articles in those files.  It allows the user
    to run searches against the data in that index.
    """
    while True:
        query = input("Query (empty query to stop): ")
        query = query.lower()                   # convert query to lowercase
        if query == '':
            break
        results = search(index, query)

        # display query results
        print("Results for query '" + query + "':")
        if results:                             # check for non-empty results list
            for i in range(len(results)):
                title = file_titles[results[i]]
                print(str(i + 1) + ".  Title: " + title + ",  File: " + results[i])
        else:
            print("No results match that query.")


def textfiles_in_dir(directory):
    """
    DO NOT MODIFY
    Given the name of a valid directory, returns a list of the .txt
    file names within it.

    Input:
        directory (string): name of directory
    Returns:
        list of (string) names of .txt files in directory
    """
    filenames = []

    for filename in os.listdir(directory):
        if filename.endswith('.txt'):
            filenames.append(os.path.join(directory, filename))

    return filenames


def main():
    """
    Usage: searchengine.py <file directory> -s
    The first argument specified should be the directory of text files that
    will be indexed/searched.  If the parameter -s is provided, then the
    user can interactively search (using the index).  Otherwise (if -s is
    not included), the index and the dictionary mapping file names to article
    titles are just printed on the console.
    """
    # Get command line arguments
    args = sys.argv[1:]

    num_args = len(args)
    if num_args < 1 or num_args > 2:
        print('Please specify directory of files to index as first argument.')
        print('Add -s to also search (otherwise, index and file titles will just be printed).')
    else:
        # args[0] should be the folder containing all the files to index/search.
        directory = args[0]
        if os.path.exists(directory):
            # Build index from files in the given directory
            files = textfiles_in_dir(directory)
            index = {}          # index is empty to start
            file_titles = {}    # mapping of file names to article titles is empty to start
            create_index(files, index, file_titles)

            # Either allow the user to search using the index, or just print the index
            if num_args == 2 and args[1] == '-s':
                do_searches(index, file_titles)
            else:
                print('Index:')
                print(index)
                print('File names -> document titles:')
                print(file_titles)
        else:
            print('Directory "' + directory + '" does not exist.')


if __name__ == '__main__':
    main()
