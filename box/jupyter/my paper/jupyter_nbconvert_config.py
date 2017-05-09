c = get_config()

#Export all the notebooks in the current directory to the sphinx_howto format.
c.NbConvertApp.notebooks = ['papper_scratch.ipynb']
c.NbConvertApp.export_format = 'pdf'

c.Exporter.template_file = 'citations'