
**************************************************
Simple Dynamic Level of Detail for Geometry Images
**************************************************

This repository includes the code for "Simple dynamic LOD for geometry images" presented at GRAPHITE '06: Proceedings of the 4th international conference on Computer graphics and interactive techniques in Australasia and Southeast Asia.

If you use this code in a publication or project, a link to or citation of both, our paper and this repository, would be appreciated.

The BibTex entries for our publication and code are:

.. code-block:: bash
    
    @inproceedings{10.1145/1174429.1174454,
    author = {Hern\'{a}ndez, Benjam\'{\i}n and Rudomin, Isaac},
    title = {Simple Dynamic LOD for Geometry Images},
    year = {2006},
    isbn = {1595935649},
    publisher = {Association for Computing Machinery},
    address = {New York, NY, USA},
    url = {https://doi.org/10.1145/1174429.1174454},
    doi = {10.1145/1174429.1174454},
    abstract = {We present a new approach for dynamic LOD processing for geometry images (GIs) in
    the graphics processing unit (GPU). A GI mipmap is constructed from a scanned 3D model,
    then a mipmap selector map is created from the camera position's information and the
    GI-mipmap. Using the mipmap selector map and the current camera position, some parts
    of the GI mipmap are discarded using a one-pass shader program that selects the mipmap
    level that must be displayed and calculates backface culling. Since our method does
    not require any extra spatial data structure, conceptually it is very simple. Since
    LOD selection is a one pass algorithm performed on the GPU, it is also fast.},
    booktitle = {Proceedings of the 4th International Conference on Computer Graphics and Interactive Techniques in Australasia and Southeast Asia},
    pages = {157–163},
    numpages = {7},
    keywords = {GPGPU, geometry image, dynamic LOD, GI mipmap},
    location = {Kuala Lumpur, Malaysia},
    series = {GRAPHITE '06}
    }

.. code-block:: bash

    @misc{SimpleDLODGI_2006,
    auhor = {Hern\'andez, Benjam\'in},
    title = {Simple Dynamic LOD for Geometry Images},
    year  = {2021},
    url = {https://github.com/benjha/simple-dlod-gi},
    note = {\url{https://github.com/benjha/simple-dlod-gi}}
   }
