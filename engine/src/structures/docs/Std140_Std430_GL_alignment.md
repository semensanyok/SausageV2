excerpt from glspec core 4.5: https://registry.khronos.org/OpenGL/specs/gl/glspec45.core.pdf

-------------------------------------------------------------------------------------
When using the std140 storage layout, structures will be laid out in buffer
storage with their members stored in monotonically increasing order based on their
location in the declaration. A structure and each structure member have a base
offset and a base alignment, from which an aligned offset is computed by rounding
the base offset up to a multiple of the base alignment. The base offset of the first
member of a structure is taken from the aligned offset of the structure itself. The
base offset of all other structure members is derived by taking the offset of the
last basic machine unit consumed by the previous member and adding one. Each
structure member is stored in memory at its aligned offset. The members of a toplevel
uniform block are laid out in buffer storage by treating the uniform block as
a structure with a base offset of zero.

1. If the member is a scalar consuming N basic machine units, the base alignment
is N.
2. If the member is a two- or four-component vector with components consuming
N basic machine units, the base alignment is 2N or 4N, respectively.
3. If the member is a three-component vector with components consuming N
basic machine units, the base alignment is 4N.
4. If the member is an array of scalars or vectors, the base alignment and array
stride are set to match the base alignment of a single array element, according
to rules (1), (2), and (3), and rounded up to the base alignment of a vec4. The
array may have padding at the end; the base offset of the member following
the array is rounded up to the next multiple of the base alignment.
5. If the member is a column-major matrix with C columns and R rows, the
matrix is stored identically to an array of C column vectors with R components
each, according to rule (4).
6. If the member is an array of S column-major matrices with C columns and
R rows, the matrix is stored identically to a row of S  C column vectors
with R components each, according to rule (4).
7. If the member is a row-major matrix with C columns and R rows, the matrix
is stored identically to an array of R row vectors with C components each,
according to rule (4).
8. If the member is an array of S row-major matrices with C columns and R
rows, the matrix is stored identically to a row of S  R row vectors with C
components each, according to rule (4).
9. If the member is a structure, the base alignment of the structure is N, where
N is the largest base alignment value of any of its members, and rounded
up to the base alignment of a vec4. The individual members of this substructure
are then assigned offsets by applying this set of rules recursively,
where the base offset of the first member of the sub-structure is equal to the
aligned offset of the structure. The structure may have padding at the end;
the base offset of the member following the sub-structure is rounded up to
the next multiple of the base alignment of the structure.
10. If the member is an array of S structures, the S elements of the array are laid
out in order, according to rule (9).

Shader storage blocks (see section 7.8) also support the std140 layout qualifier,
as well as a std430 qualifier not supported for uniform blocks. When using
the std430 storage layout, shader storage blocks will be laid out in buffer storage
identically to uniform and shader storage blocks using the std140 layout, except
that the base alignment and stride of arrays of scalars and vectors in rule 4 and of
structures in rule 9 are not rounded up a multiple of the base alignment of a vec4.

-------------------------------------------------------------------------------------

# Notes:
1) on SSBO padding, between glsl and Sausage structures.\
(item 9. and footer). Noted because used extensively.
	- std140: 
			`
			  and rounded up to the base alignment of a vec4.
			 `
	- std430: 
			`
	          -//- identically to ... std140 layout, except
			  that the base alignment and stride of ... and of structures in rule 9 
			  are not rounded up a multiple of the base alignment of a vec4.
			 `
 