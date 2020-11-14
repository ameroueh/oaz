import matplotlib.patches as patches
import matplotlib.pyplot as plt


def view_board(board):
    """
    View a board.

    Adapted from
    https://stackoverflow.com/questions/24563513/drawing-a-go-board-with-matplotlib
    """
    width = 7
    height = 6
    fig = plt.figure(figsize=[width, height])

    ax = fig.add_subplot(111)
    ax.add_patch(
        patches.Rectangle(
            xy=(0, 0),
            width=width,
            height=height,
            fill=True,
            facecolor=(1, 1, 0.8),
        )
    )
    for x in range(width + 1):
        ax.plot([x, x], [0, height], "k")
    for y in range(height + 1):
        ax.plot([0, width], [y, y], "k")

    ax.set_position([0, 0, 1, 1])
    ax.set_xlim(-1, width + 1)
    ax.set_ylim(-1, height + 1)

    for x in range(width):
        for y in range(height):
            if board[y][x][0] == 1.0:
                ax.plot(
                    x + 0.5,
                    y + 0.5,
                    "o",
                    markersize=30,
                    markeredgecolor=(0.5, 0.5, 0.5),
                    markerfacecolor="k",
                    markeredgewidth=2,
                )
            elif board[y][x][1] == 1.0:
                ax.plot(
                    x + 0.5,
                    y + 0.5,
                    "o",
                    markersize=30,
                    markeredgecolor=(0.5, 0.5, 0.5),
                    markerfacecolor="w",
                    markeredgewidth=2,
                )
    plt.axis("off")
