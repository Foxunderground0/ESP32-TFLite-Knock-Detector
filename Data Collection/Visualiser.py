import os
import glob
import matplotlib.pyplot as plt

# Set the number of plots per page
plots_per_page = 9

# Get a list of all the data files in the True directory
true_files = glob.glob('Data Collection/Data/Truea/*.txt')

# Get a list of all the data files in the False directory
false_files = glob.glob('Data Collection/Data/False/*.txt')

# Combine the true and false files into one list
all_files = true_files + false_files

# Define a function to plot the data from a single file


def plot_file(filename):
    # Load the data from the file
    data = []
    with open(filename, 'r') as f:
        for line in f:
            data.append(float(line.strip()))

    # Plot the data
    plt.plot(data)

    # Set the axis labels and title
    plt.xlabel('Sample')
    plt.ylabel('Value')
    plt.title(filename)

    # Set the y-axis limits
    plt.ylim([0.2, 1.8])

# Define a function to show a page of plots


def show_plots(page_num):
    # Clear the current plot
    plt.clf()

    # Get the filenames for the plots on this page
    start_index = page_num * plots_per_page
    end_index = min(start_index + plots_per_page, len(all_files))
    filenames = all_files[start_index:end_index]

    # Plot each file on the page
    for i, filename in enumerate(filenames):
        plt.subplot(3, 3, i + 1)
        plot_file(filename)

    # Show the plot
    plt.show(block=False)


# Show the first page of plots
current_page = 0
show_plots(current_page)

# Define a function to handle key presses


def on_key_press(event):
    global current_page
    if event.key == ' ':
        # Move to the next page of plots
        current_page += 1
        if current_page * plots_per_page >= len(all_files):
            current_page = 0
        show_plots(current_page)


# Connect the key press event to the on_key_press function
plt.connect('key_press_event', on_key_press)

# Wait for key presses
plt.show()
