def edge_definer(bin_mat):
    rows = len(bin_mat[:,1])
    cols = len(bin_mat[1,:])
    matrix_edge = np.zeros([rows,cols])
    for i in range(rows):
        temp_val = 0
        for j in range(cols):
            if temp_val != bin_mat[i,j]:
                matrix_edge[i,j] = 1
                if temp_val == 0:
                    temp_val = 1
                else:
                    temp_val = 0
    for j in range(cols):
        temp_val = 0
        for i in range(rows):
            if temp_val != bin_mat[i,j]:
                matrix_edge[i,j] = 1
                if temp_val == 0:
                    temp_val = 1
                else:
                    temp_val = 0
    return matrix_edge
def edge_finder(im):
    bin_mat1 = filter_color(im,180,253,100,150,130,140)  #orange pole and chairs
    bin_mat2 = filter_color(im,70,120,150,160,100,120)   #Blue chair
    bin_mat3 = filter_color(im,100,200,90,130,160,240) #Orange 
    
    bin_mat_tot = bin_mat1+bin_mat2+bin_mat3   
    matrix_edge = edge_definer(bin_mat_tot)
    
    plt.figure()
    plt.imshow(matrix_edge)
    plt.title('Edges only')     
    return matrix_edge
